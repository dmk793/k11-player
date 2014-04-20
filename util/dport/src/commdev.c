/*
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <termios.h>
#include <errno.h>
#include "commdev.h"
#include "errcode.h"

#define COMMDEV_DEBUG

#ifdef COMMDEV_DEBUG
    #define COMMDEV_DEBUGF(...) printf(__VA_ARGS__)
#else
    #define COMMDEV_DEBUGF(...)
#endif

#define BAUDRATE B115200

/*
 *
 */
void commdev_init()
{
    return;
}

/*
 * open USB device
 *
 * ARGS
 *     commdev    pointer to commdev_t structure
 */
void commdev_open(struct commdev_t *commdev, char *dev)
{
    struct termios tios;

    commdev->fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK); 
    if (commdev->fd < 0)
    {
	perror(dev);
	exit(ERRCODE_DEV_OPEN);
    }

    tcgetattr(commdev->fd, &tios);

    /* save old setting */
    memcpy(&commdev->otios, &tios, sizeof(struct termios));
    
    /*
     * termios_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
     *                 | INLCR | IGNCR | ICRNL | IXON);
     * termios_p->c_oflag &= ~OPOST;
     * termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
     * termios_p->c_cflag &= ~(CSIZE | PARENB);
     * termios_p->c_cflag |= CS8;
     */
//    cfmakeraw(&tios);

    tios.c_iflag = IGNPAR | IGNBRK;
    tios.c_oflag = 0;
    tios.c_cflag = CLOCAL | CS8 | CREAD;
    tios.c_lflag = 0;
    tios.c_cc[VMIN]   = 1;
    tios.c_cc[VTIME]  = 0;

    cfsetospeed(&tios, BAUDRATE);
    cfsetispeed(&tios, BAUDRATE);
    tcsetattr(commdev->fd, TCSAFLUSH, &tios);
    tcflush(commdev->fd, TCIOFLUSH);
}

/*
 *
 */
void commdev_close(struct commdev_t *commdev)
{
    tcsetattr(commdev->fd, TCSAFLUSH, &commdev->otios);
    close(commdev->fd);
}

/*
 * RETURN
 *     count of bytes sended
 *
 */
int commdev_send(struct commdev_t *commdev, uint8 *buf, int len, int timeout)
{
    fd_set wset;
    int n, wr, sl;
    struct timeval tv; 

    n = len;
    while (len)
    {
        FD_ZERO(&wset);
        FD_SET(commdev->fd, &wset);
        tv.tv_sec  = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        sl = select(commdev->fd + 1, NULL, &wset, NULL, &tv);
        if (sl == 0)
            return 0;
        if (sl < 0)
        {
            if (errno == EINTR)
            {
                printf("select EINTR\n");
                return -1;
            }

            perror(__FUNCTION__);
            exit(ERRCODE_COMMDEV_SELECT_SEND);
        }

        wr = write(commdev->fd, buf, len);
        if (wr < 0)
        {
            perror(__FUNCTION__);
            exit(ERRCODE_COMMDEV_SEND);
        }
        len -= wr;
        buf += wr;
    }

    return n;
}

/*
 * RETURN
 *     zero on timeout, count of bytes received on success, -1 on EINTR, exit from program
 *     on errors
 */
int commdev_recv(struct commdev_t *commdev, uint8 *buf, int len, int timeout)
{
    fd_set rset;
    struct timeval tv; 
    int sl, rd;

    FD_ZERO(&rset);
    FD_SET(commdev->fd, &rset);
    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    sl = select(commdev->fd + 1, &rset, NULL, NULL, &tv);
    if (sl == 0)
        return 0;
    if (sl < 0)
    {
        if (errno == EINTR)
        {
            printf("select EINTR\n");
            return -1;
        }

        perror(__FUNCTION__);
        exit(ERRCODE_COMMDEV_SELECT_RECV);
    }

    rd = read(commdev->fd, buf, len);
    if (rd < 0)
    {
        if (errno == EINTR)
        {
            printf("EINTR\n");
            return -1;
        }
        perror(__FUNCTION__);
        exit(ERRCODE_COMMDEV_RECV);
    }

    return rd;
}

/*
 *
 */
void commdev_exit()
{
    return;
}

