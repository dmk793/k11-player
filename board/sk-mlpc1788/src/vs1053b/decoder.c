/* 
 *     This file is part of K11, hardware multimedia player.
 * 
 * Copyright (C) 2014 Dmitry Kobylin
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <string.h>
#include <os.h>
#include <debug.h>
#include <stimer.h>
#include <util.h>
#include "vs1053b.h"
#include "decoder.h"
#include "../player/player.h"
#include "../fat_io_lib/fat_filelib.h"


#define DEBUG_DECODER

#ifdef DEBUG_DECODER
    #define DPRINT(fmt, ...)  dprint(fmt, __VA_ARGS__)
#else
    #define DPRINT(fmt, ...)
#endif


#define DECODER_FEED_END        0
#define DECODER_FEED_STOPPED    1
static int decoder_feed();
#define DECODER_SEND_POSITION_NORMAL   0
#define DECODER_SEND_POSITION_NONE     1
static void decoder_send_position(int none);
#define DECODER_SET_PLAY_SPEED_NORMAL   0
#define DECODER_SET_PLAY_SPEED_FAST     1
static void decoder_set_play_speed(int fast);
#define DECODER_PROCMSG_NOP     0
#define DECODER_PROCMSG_STOP    1
static int decoder_process_msg();

#ifdef DEC_TEST_GPIO
const struct gpio_t dec_testpins[2] = {
    {LPC_GPIO3, (1 << 28)}, /* MP5, P3[28] */
    {LPC_GPIO3, (1 << 29)}, /* MP11, P3[29] */
};

#endif

struct decoder_t {
    uint32 flen;    /* whole file length */
    uint32 fpos;    /* current decode position */

    uint32 posto;   /* send position timer */
    uint32 intrto;  /* interrupt timer */

#define DECODER_CMD_PAUSE    (1 << 0) /* pause playback */
#define DECODER_CMD_FASTP    (1 << 1) /* fast playback */
#define DECODER_CMD_INTRP    (1 << 2) /* interrupt playback */
    uint32 cmd;
    int volume;
};


static struct decoder_t decoder;

/*
 *
 */
void decoder_task()
{
    struct decoder_msg_t dmsg;
    struct player_msg_t pmsg;
    BASE_TYPE msglen;
    int play;

    os_event_wait(&player.event, PLAYER_EVENT_INIT, OS_FLAG_NONE, OS_WAIT_FOREVER);
//    vs1053b_set_volume(30);

#ifdef DEC_TEST_GPIO
    gpio_setdir(DECTEST_GPIO0, GPIO_DIR_OUTPUT);
    gpio_setdir(DECTEST_GPIO1, GPIO_DIR_OUTPUT);

    gpio_drive(DECTEST_GPIO0, 0);
    gpio_drive(DECTEST_GPIO1, 0);
#endif

    while (1)
    {
        /* cleanup all commands */
        decoder.cmd = 0;

        vs1053b_set_low_fclk();

        play = 0;
        do
        {
            if (os_queue_remove(player.qdecoder, OS_FLAG_NONE, OS_WAIT_FOREVER,
                        &dmsg, &msglen) == OS_ERR_NONE)
            {
                switch (dmsg.id)
                {
                    case DECODER_MSG_ID_PLAY:
                        decoder.flen  = dmsg.play.flen;
                        decoder.fpos  = 0;
                        stimer_settime(&decoder.posto);
                        play = 1;

                        /* send message that we are started playback */
                        pmsg.id = PLAYER_MSG_ID_DEC_PLAY;
                        PLAYER_SEND_MSG_BLOCKING(&pmsg, pmsg_empty_t);

                        break;
                    case DECODER_MSG_ID_INTR_PLAY:
                        /* FALLTHROUGH */
                    case DECODER_MSG_ID_STOP:
                        /* send confirm that we are idle and player can initialize playback */
                        pmsg.id = PLAYER_MSG_ID_DEC_STOPPED;
                        PLAYER_SEND_MSG(&pmsg, pmsg_empty_t);
                        break;
                    case DECODER_MSG_ID_VOLUME:
                        {
                            int value;
                            value = decoder_set_volume(dmsg.volume.value);

                            /* send real volume information */
                            pmsg.id           = PLAYER_MSG_ID_DEC_VOLUME;
                            pmsg.volume.value = value;
                            PLAYER_SEND_MSG_BLOCKING(&pmsg, pmsg_volume_t);
                        }
                        break;
                    default:
                        DEBUG_WMSGF("0. odd message", "1xn", dmsg.id);
                } 
            }
        } while (!play);

        decoder_set_play_speed(DECODER_SET_PLAY_SPEED_NORMAL);
        vs1053b_set_decode_time(0);

        /* data to vs1053 can be feeded on higher clock rate */
        vs1053b_set_hi_fclk();

        /* 
         * query next track or send stopped message
         */
        if (decoder_feed() == DECODER_FEED_STOPPED)
        {
            pmsg.id = PLAYER_MSG_ID_DEC_STOPPED;
            PLAYER_SEND_MSG(&pmsg, pmsg_empty_t);
        } else {
            /* NOTE send in blocking mode */
            pmsg.id = PLAYER_MSG_ID_DEC_NEXTTRACK;
            PLAYER_SEND_MSG_BLOCKING(&pmsg, pmsg_empty_t);
        }

        vs1053b_cancel();
    }
}

/*
 * send position information to player
 */
static void decoder_send_position(int none)
{
    struct player_msg_t pmsg;

#define DECODER_NORMAL_SEND_POSITION_TO    1000
#define DECODER_FAST_SEND_POSITION_TO      200
#define DECODER_SEND_POSITION_TO (decoder.cmd & DECODER_CMD_FASTP ? DECODER_FAST_SEND_POSITION_TO : DECODER_NORMAL_SEND_POSITION_TO)

    if (none)
    {
        pmsg.id = PLAYER_MSG_ID_DEC_POSITION;
        pmsg.position.time  = 0;
        pmsg.position.value = 0;
        PLAYER_SEND_MSG(&pmsg, pmsg_position_t);
    } else if (stimer_deltatime(decoder.posto) >= DECODER_SEND_POSITION_TO) {
        stimer_settime(&decoder.posto);
        pmsg.id = PLAYER_MSG_ID_DEC_POSITION;

        vs1053b_set_low_fclk();
        pmsg.position.time = vs1053b_get_decode_time();
        vs1053b_set_hi_fclk();

        pmsg.position.value = decoder.fpos * 100 / decoder.flen;
        PLAYER_SEND_MSG(&pmsg, pmsg_position_t);
    }
}

/*
 * set playback speed
 */
#define DECODER_NORMAL_PLAY_SPEED    1
#define DECODER_FAST_PLAY_SPEED      16
static void decoder_set_play_speed(int fast)
{
    /* TODO should mute be used while we are in fast playback? */

    vs1053b_set_low_fclk();
    vs1053b_set_play_speed(fast ? DECODER_FAST_PLAY_SPEED : DECODER_NORMAL_PLAY_SPEED);
    vs1053b_set_hi_fclk();
}

/*
 *
 */
int
decoder_set_volume(int value)
{
    int ret;

    vs1053b_set_low_fclk();
    ret = vs1053b_set_volume(value);
    vs1053b_set_hi_fclk();

    return ret;
}


/*
 * feed vs1053 with data
 *
 * RETURN
 *    1 if playback was stopped, 0 if full file was exhausted
 */
static int decoder_feed()
{
    struct fcache_entry_t *entry;
    uint8 *buf;
    int n;
    int drained;
    int ret;

    /* NOTE precache */
    player_fcache_fill();

//    decoder_send_position(DECODER_SEND_POSITION_NONE);

    ret = DECODER_FEED_END;
    while (1)
    {
        entry = player_fcache_get(&drained);
        if (!entry)
        {
            /*
             * If file was drained and cache underflow was occured we can stop
             * decoding.
             */
            if (drained)
            {
                ret = DECODER_FEED_END;
                goto out;
            }

            DEBUG_WMSG("cache underflow");

            /* fill cache */
            player_fcache_fill();

//            os_event_clear(&player.event, FILE_CACHE_FILLED_EVENT);
//            os_event_wait(&player.event, FILE_CACHE_FILLED_EVENT, OS_FLAG_CLEAR, OS_MS2TICK(100));

            /* process messages from player */
            if (decoder_process_msg() == DECODER_PROCMSG_STOP)
            {
                ret = DECODER_FEED_STOPPED;
                goto out;
            }

            continue;
        }

        buf = entry->buf;

#define FEED_PORTION    32
        while (entry->len)
        {
            n = entry->len;
            if (n > FEED_PORTION)
                n = FEED_PORTION;

#ifdef DEC_TEST_GPIO
            gpio_drive(DECTEST_GPIO0, 1);
#endif
          vs1053b_writesdi(buf, n);
#ifdef DEC_TEST_GPIO
            gpio_drive(DECTEST_GPIO0, 0);
#endif

            buf          += n;
            entry->len   -= n;
            decoder.fpos += n;

            if (!vs1053b_check_dreq())
            {
                /* fill cache */
                player_fcache_fill();

                /* send position information to player */
                decoder_send_position(DECODER_SEND_POSITION_NORMAL);

                /* process messages from player */
                if (decoder_process_msg() == DECODER_PROCMSG_STOP)
                {
                    ret = DECODER_FEED_STOPPED;
                    goto out;
                }

#ifdef DEC_TEST_GPIO
                gpio_drive(DECTEST_GPIO1, 1);
#endif
                vs1053b_wait_dreq();
#ifdef DEC_TEST_GPIO
                gpio_drive(DECTEST_GPIO1, 0);
#endif
            }
        }
        player_fcache_remove();
    }

out:
    decoder_send_position(DECODER_SEND_POSITION_NONE);
    return ret;
}


/*
 * Return 1 if decoder should be stopped
 */
static int decoder_process_msg()
{
    struct player_msg_t pmsg;
    struct decoder_msg_t dmsg;
    BASE_TYPE qres;

#define MESSAGE_CHECK_TO         500    /* ms */
#define PLAYBACK_INTERRUPT_TO   1000    /* ms */

    while (1)
    {
        if (decoder.cmd & (DECODER_CMD_PAUSE | DECODER_CMD_INTRP))
            qres = os_queue_remove(player.qdecoder, OS_FLAG_NONE, OS_MS2TICK(MESSAGE_CHECK_TO), &dmsg, NULL);
        else
            qres = os_queue_remove(player.qdecoder, OS_FLAG_NOWAIT, OS_WAIT_FOREVER, &dmsg, NULL);
        if (qres == OS_ERR_NONE)
        {
            switch (dmsg.id)
            {
                case DECODER_MSG_ID_STOP:
                    dprint("sn", "stop");
                    return DECODER_PROCMSG_STOP; /* NOTE */
                case DECODER_MSG_ID_PAUSE:
                    if (decoder.cmd & DECODER_CMD_PAUSE)
                        BITMASK_CLEAR(decoder.cmd, DECODER_CMD_PAUSE);
                    else
                        BITMASK_SET(decoder.cmd, DECODER_CMD_PAUSE);
                    break;
                case DECODER_MSG_ID_FAST_PLAY:
                    if (decoder.cmd & DECODER_CMD_FASTP)
                        BITMASK_CLEAR(decoder.cmd, DECODER_CMD_FASTP);
                    else
                        BITMASK_SET(decoder.cmd, DECODER_CMD_FASTP);

                    decoder_set_play_speed(decoder.cmd & DECODER_CMD_FASTP ?
                            DECODER_SET_PLAY_SPEED_FAST : DECODER_SET_PLAY_SPEED_NORMAL);
                    break;
                case DECODER_MSG_ID_INTR_PLAY:
                    BITMASK_SET(decoder.cmd, DECODER_CMD_INTRP);
                    stimer_settime(&decoder.intrto);
                    break;
                case DECODER_MSG_ID_VOLUME:
                    {
                        int value;
                        value = decoder_set_volume(dmsg.volume.value);

                        /* send real volume information */
                        pmsg.id           = PLAYER_MSG_ID_DEC_VOLUME;
                        pmsg.volume.value = value;
                        PLAYER_SEND_MSG_BLOCKING(&pmsg, pmsg_volume_t);
                    }
                    break;
                default:
                    DEBUG_WMSGF("1. odd message", "1xn", dmsg.id);
            }
        }

        if (decoder.cmd & (DECODER_CMD_PAUSE | DECODER_CMD_INTRP))
        {
            /* send confirm that we are paused */
            if (qres == OS_ERR_NONE)
            {
                pmsg.id = PLAYER_MSG_ID_DEC_PAUSED;
                PLAYER_SEND_MSG_BLOCKING(&pmsg, pmsg_empty_t);
            }

            if (decoder.cmd & DECODER_CMD_INTRP)
            {
                if (stimer_deltatime(decoder.intrto) >= PLAYBACK_INTERRUPT_TO)
                    BITMASK_CLEAR(decoder.cmd, DECODER_CMD_INTRP);
            }

//            os_wait_ms(150);
        } else {
            /* send message that we resume playback */
            if (qres == OS_ERR_NONE)
            {
                pmsg.id = PLAYER_MSG_ID_DEC_PLAY;
                PLAYER_SEND_MSG_BLOCKING(&pmsg, pmsg_empty_t);
            }
            break;
        }
    }

    return DECODER_PROCMSG_NOP;
}

