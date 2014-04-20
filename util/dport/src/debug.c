#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <tcl.h>
#include "linenoise.h"
#include "comm.h"
#include "debug.h"
#include "errcode.h"
#include "tcl_interface.h"
#include "dport_proto.h"
#include "darm/darm.h"

struct debug_t debug;
static struct sigaction sact;
static char lastcmd[PATH_MAX] = {0};

static void sigact(int sig, siginfo_t *sinf, void *context);
static void debug_lncompletion(const char *buf, linenoiseCompletions *lc);

static int debug_cmd_break(struct debug_t *debug);

/*
 *
 */
int debug_loop()
{
    int rd, i;
    char *line;

    /* hook signal handler */
    memset(&sact, 0, sizeof(sact));
    sact.sa_sigaction = sigact;
    sact.sa_flags = SA_SIGINFO | SA_RESTART;
    if (sigaction(SIGINT, &sact, NULL) == -1)
    {
        perror("sigaction SIGINT");
        return ERRCODE_SIGNAL_INSTALL;
    }

    linenoiseSetCompletionCallback(debug_lncompletion);

    while (debug.run)
    {
        /* ouput to screen everething that was received from debug port */
        rd = commdev_recv(debug.commdev, debug.rx.buf, DEBUG_BUF_SIZE, 1000);
        if (rd > 0)
        {
            for (i = 0; i < rd; i++)
                printf("%c", debug.rx.buf[i]);
            fflush(stdout);
            continue;
        }
        if (rd == 0)
            continue;

        /*
         * "commdev_recv" return -1, that means that ctrl+c was hit.
         * Send break to target.
         */
        if (debug_cmd_break(&debug) < 0)
        {
            fprintf(stderr, "(E) break failed\n");
            continue;
        }

        /* enter command loop */
        while(!debug.cont)
        {
            char *cmd;

            line = linenoise("dport> ");
            if (!line)
                continue;

            if (line[0])
            {
                if (strcmp(lastcmd, line))
                {
                    linenoiseHistoryAdd(line);
                    strcpy(lastcmd, line);
                }
            }
            cmd = lastcmd;
            if (cmd[0])
            {
                /* process command by TCL */
                if (Tcl_Eval(debug.interp, cmd) != TCL_OK)
                {
                    char *result;

                    result = Tcl_GetStringFromObj(Tcl_GetObjResult(debug.interp), NULL);
                    if (result)
                    {
                        fprintf(stderr, "*** ERROR ***\n");
    //                    fprintf(stderr, "%s\n", result);
                        Tcl_Eval(debug.interp, "puts stderr $::errorInfo");
                        fprintf(stderr, "*************\n");
                    }
                }
            }

            free(line);
        }
        debug.cont = 0; /* NOTE */
    }

    return 0;
}

/*
 * signal handler
 */
static void sigact(int sig, siginfo_t *sinf, void *context)
{
    switch (sig)
    {
        case SIGINT:
//            printf("SIGINT, %d\n", sig);
//            kill(getpid(), sig);
            break;
        default:
            printf("unknown signal received\n");
            break;
    }
}

/*
 * process linenoise completion
 */
static void debug_lncompletion(const char *buf, linenoiseCompletions *lc)
{
    /* 
     * There is no need to add completions for
     * some commands since they are only return result
     * to TCL interpreter. This commands are:
     *     readmem
     *     readreg
     */

    /* TODO subcommands completion */
    if (buf[0] == 'c') {
        linenoiseAddCompletion(lc, "cont");
        return;
    }
    if (buf[0] == 'd') {
        linenoiseAddCompletion(lc, "disasm ");
        return;
    }
    if (buf[0] == 'h') {
        linenoiseAddCompletion(lc, "help ");
        return;
    }
    if (buf[0] == 'p') {
        linenoiseAddCompletion(lc, "printmem ");
        return;
    }
    if (buf[0] == 'q') {
        linenoiseAddCompletion(lc, "quit");
        return;
    }
    if (buf[0] == 'r') {
        if (strncmp(buf, "regs", strlen(buf)) == 0)
            linenoiseAddCompletion(lc, "regs");
        if (strncmp(buf, "reset", strlen(buf)) == 0)
            linenoiseAddCompletion(lc, "reset");
        return;
    }
    if (buf[0] == 's') {
        linenoiseAddCompletion(lc, "step");
        return;
    }
    if (buf[0] == 'u') {
        if (strncmp(buf, "userf", strlen(buf)) == 0)
            linenoiseAddCompletion(lc, "userf");
        return;
    }
    if (buf[0] == 'w') {
        if (strncmp(buf, "writeimage", strlen(buf)) == 0)
            linenoiseAddCompletion(lc, "writeimage ");
        if (strncmp(buf, "writeflash", strlen(buf)) == 0)
            linenoiseAddCompletion(lc, "writeflash ");
        if (strncmp(buf, "writemem", strlen(buf)) == 0)
            linenoiseAddCompletion(lc, "writemem ");
        if (strncmp(buf, "writereg", strlen(buf)) == 0)
            linenoiseAddCompletion(lc, "writereg ");
    }
}

/*
 * get buffer size of target
 *
 * RETURN
 *     0 on success, -1 on error
 */
int debug_cmd_get_bufsize(struct debug_t *debug)
{
    union dport_resp_t *resp; 

    comm_mkreq(&debug->tx, DPORT_CMD_REQ_BUFSIZE, NULL, 0);
    if (comm_txrx(debug) < 0)
        return -1;

    resp = (union dport_resp_t *)&debug->rx.buf[2];
    debug->bufsize = resp->bufsize.length;
    printf("buffer size of target is %u\n", debug->bufsize);

    if (debug->bufsize > DEBUG_BUF_SIZE)
    {
        fprintf(stderr, "(E) size of target buffer too big, %s", __FUNCTION__);
        return -1;
    }

    return 0;
}

/*
 * send break command to target
 *
 * RETURN
 *     0 on success, -1 on error
 */
static int debug_cmd_break(struct debug_t *debug)
{
    union dport_resp_t *resp; 

    comm_mkreq(&debug->tx, DPORT_CMD_REQ_BREAK, NULL, 0);
    if (comm_txrx(debug) < 0)
        return -1;

    resp = (union dport_resp_t *)&debug->rx.buf[2];
    printf("break at %08X\n", resp->brk.addr);

    return 0;
}

/*
 * send continue command to target
 *
 * RETURN
 *     0 on success, -1 on error
 */
int debug_cmd_cont(struct debug_t *debug)
{
    comm_mkreq(&debug->tx, DPORT_CMD_REQ_CONTINUE, NULL, 0);
    if (comm_txrx(debug) < 0)
        return -1;

    debug->cont = 1;

    return 0;
}

/*
 * send step command to target
 *
 * RETURN
 *     0 on success, -1 on error
 */
int debug_cmd_step(struct debug_t *debug)
{
    comm_mkreq(&debug->tx, DPORT_CMD_REQ_STEP, NULL, 0);
    if (comm_txrx(debug) < 0)
        return -1;

    return 0;
}



/*
 * send continue command to target
 *
 * RETURN
 *     0 on success, -1 on error
 *
 */
int debug_cmd_reset(struct debug_t *debug)
{
    comm_mkreq(&debug->tx, DPORT_CMD_REQ_RESET, NULL, 0);
    if (comm_txrx(debug) < 0)
        return -1;

    return 0;
}

/*
 * read register from target
 *
 * ARGS
 *     n       register number
 *     value   pointer where value will be stored
 *
 * RETURN
 *     0 on success, -1 on error
 *
 */
int debug_cmd_read_reg(struct debug_t *debug, int n, uint32 *value)
{
    union dport_req_t *req; 
    union dport_resp_t *resp; 

    req  = &debug->req;
    resp = (union dport_resp_t *)&debug->rx.buf[2];

    req->read_reg.regn = n;
    comm_mkreq(&debug->tx, DPORT_CMD_REQ_READREG, (uint8*)req, sizeof(struct dport_req_read_reg_t));
    if (comm_txrx(debug) < 0)
        return -1;

    if (resp->read_reg.regn != n)
    {
        fprintf(stderr, "(E) register mismatch, %s\n", __FUNCTION__);
        return -1;
    }

    *value = resp->read_reg.value;

    return 0;
}

/*
 * write register of target
 *
 * ARGS
 *     n       register number
 *     value   value to write
 *
 * RETURN
 *     0 on success, -1 on error
 *
 */
int debug_cmd_write_reg(struct debug_t *debug, int n, uint32 value)
{
    union dport_req_t  *req; 
    union dport_resp_t *resp; 

    req  = &debug->req;
    resp = (union dport_resp_t *)&debug->rx.buf[2];

    req->write_reg.regn = n;
    req->write_reg.value = value;
    comm_mkreq(&debug->tx, DPORT_CMD_REQ_WRITEREG, (uint8*)req, sizeof(struct dport_req_write_reg_t));
    if (comm_txrx(debug) < 0)
        return -1;

    if (resp->write_reg.regn != n)
    {
        fprintf(stderr, "(E) register mismatch, %s\n", __FUNCTION__);
        return -1;
    }

    return 0;
}

/*
 * read memory of target
 *
 * ARGS
 *     addr    register number
 *     len     count of bytes to read
 *     buf     buffer where data will be placed
 *
 * RETURN
 *     0 on success, -1 on error
 *
 */
int debug_cmd_read_mem(struct debug_t *debug, uint32 addr, uint32 len, uint8 *buf)
{
    union dport_req_t *req; 
    union dport_resp_t *resp; 
    uint32 blocklen, n;

    blocklen  = DPORT_BINARY_PAYLOAD_SIZE2( debug->bufsize);
    blocklen -=  sizeof(struct dport_req_read_mem_t);

    req  = &debug->req;
    resp = (union dport_resp_t *)&debug->rx.buf[2];
    while (len)
    {
        n = blocklen;
        if (n > len)
            n = len;

        req->read_mem.offset = addr;
        req->read_mem.length = n;

        comm_mkreq(&debug->tx, DPORT_CMD_REQ_READMEM, (uint8*)req, sizeof(struct dport_req_read_mem_t));
        if (comm_txrx(debug) < 0)
            return -1;

        if (resp->read_mem.offset != addr || resp->read_mem.length != n)
        {
            fprintf(stderr, "(E) offset/length mismatch, %s\n", __FUNCTION__);
            return -1;
        }

        memcpy(buf, resp->read_mem.data, n);

        len  -= n;
        buf  += n;
        addr += n;
    }

    return 0;
}

/*
 *
 */
static inline void debug_print_percent(uint32 total, uint32 remain)
{
#define BACK_CHARS "\b\b\b\b"
#define PERCENT_GAP "    "
    if (remain == 0)
        printf(BACK_CHARS "%-2u %%", 100);
    else
        printf(BACK_CHARS "%-2u %%", (total - remain) * 100 / total);
    fflush(stdout);
}

/*
 * write memory of target
 *
 * ARGS
 *     addr    register number
 *     len     count of bytes to read
 *     buf     buffer with data to write
 *
 * RETURN
 *     0 on success, -1 on error
 *
 */
int debug_cmd_write_mem(struct debug_t *debug, uint32 addr, uint32 len, uint8 *buf)
{
    union dport_req_t *req; 
    union dport_resp_t *resp; 
    uint32 blocklen, n;
    int cycle;
    uint32 total; 

    blocklen = DPORT_BINARY_PAYLOAD_SIZE2(debug->bufsize);
    blocklen -= 8 /* XXX offset + length */;

    req  = &debug->req;
    resp = (union dport_resp_t *)&debug->rx.buf[2];

    total = len;
    cycle = 0;

    if (total > 1024)
        printf("write memory at 0x%08X ... " PERCENT_GAP, addr);
    while (len)
    {
        n = blocklen;
        if (n > len)
            n = len;

        req->write_mem.offset = addr;
        req->write_mem.length = n;
        memcpy(req->write_mem.data, buf, n);

        comm_mkreq(&debug->tx, DPORT_CMD_REQ_WRITEMEM, (uint8*)req,
                8 /* XXX offset + length */ + n);
        if (comm_txrx(debug) < 0)
            return -1;

        if (resp->write_mem.offset != addr || resp->write_mem.length != n)
        {
            fprintf(stderr, "(E) offset/length mismatch, %s\n", __FUNCTION__);
            return -1;
        }

        len  -= n;
        buf  += n;
        addr += n;

        /* print progress */
        if (total > 1024)
        {
            if ((cycle++ % 8) == 0 || len == 0)
            {
                debug_print_percent(total, len);
            }
        }
    }
    if (total > 1024)
        printf("\n");

    return 0;
}

/*
 * print disassembler output to screen
 */
int debug_cmd_disasm(struct debug_t *debug, uint32 addr, int ninst)
{
    uint32 data;
    int len;
    darm_t d; darm_str_t str;

    while (ninst--)
    {
        if (debug_cmd_read_mem(debug, addr, 4, (uint8*)&data) < 0)
        {
            fprintf(stderr, "(E) command \"disasm\" failed");
            return -1;
        }

        /* check 16 or 32-bit instruction */
        switch ((data >> 11) & 0x1f)
        {
            case 0b11101:
            case 0b11110:
            case 0b11111:
                len = 4;
                break;
            default:
                len = 2;
        }

        printf("%08X" PRINT_TABSTOP, addr);
        if (len == 2)
        {
            printf("%04X      ", data & 0xffff);
            if(darm_thumb_disasm(&d, data & 0xffff) == 0
                    && darm_str2(&d, &str, 1) == 0)
                printf(PRINT_TABSTOP "%s" PRINT_NEWLINE, str.total);
            else
                printf("-- disassembler error --"PRINT_NEWLINE);

            addr += 2;
        } else {
            printf("%04X %04X ", data & 0xffff, data >> 16);
            if(darm_thumb2_disasm(&d, data & 0xffff, data >> 16) == 0
                    && darm_str2(&d, &str, 1) == 0)
                printf(PRINT_TABSTOP "%s" PRINT_NEWLINE, str.total);
            else
                printf("-- disassembler error --"PRINT_NEWLINE);

            addr += 4;
        }
    }

    return 0;
}

/*
 * get size of target's flash buffer
 */
int debug_cmd_get_flash_bufsize(struct debug_t *debug, uint32 *fbsize)
{
    union dport_resp_t *resp; 

    resp = (union dport_resp_t *)&debug->rx.buf[2];
    comm_mkreq(&debug->tx, DPORT_CMD_REQ_FLASH_BUFSIZE, NULL, 0);
    if (comm_txrx(debug) < 0)
    {
        fprintf(stderr, "(E) failed to get size of target's flash buffer\n");
        return -1;
    }
    *fbsize = resp->flash_bufsize.size;

    /* sanity check */
    if (*fbsize > (DPORT_BINARY_PAYLOAD_SIZE2(debug->bufsize) - DPORT_CMD_FLASH_ARG_SIZE))
    {
        fprintf(stderr, "(E) returned size of flash buffer too long\n");
        return -1;
    }

    return 0;
}

/*
 * write binary image from file to target's flash
 */
int debug_cmd_write_image(struct debug_t *debug, uint32 offset, char *file, int length)
{
    int fd;
    int ret;
    int rd;
    uint32 fbsize;
    union dport_req_t *req; 
    union dport_resp_t *resp; 
    uint32 len;
    uint32 fsize;
    struct stat statbuf;
    int cycle;

    if (debug_cmd_get_flash_bufsize(debug, &fbsize) < 0)
        return -1;

    req  = &debug->req;
    resp = (union dport_resp_t *)&debug->rx.buf[2];

    /* open file */
    fd = open(file, O_RDONLY); 
    if (fd < 0)
    {
	perror(file);
        return -1;
    }

    /* XXX -1 - whole file */
    len = (uint32)length;
    if (fstat(fd, &statbuf) == 0)
    {
        if (statbuf.st_size < len)
            fsize = statbuf.st_size;
        else
            fsize = len;
        if (len > fsize)
            len = fsize;
    } else {
        fsize = length;
        perror("failed to get stat of file");
    }

    ret   = 0;
    cycle = 0;
    printf("write flash at 0x%08X ... " PERCENT_GAP, offset);
    while (len)
    {
        rd = fbsize;
        if (rd > len)
            rd = len;

        rd = read(fd, req->flash.data, rd);
        if (rd < 0)
        {
            fprintf(stderr, "(E) failed to read file\n");
            ret = -1;
            goto out;
        }

        if (rd == 0)
            goto end;

        req->flash.offset = offset;
        req->flash.length = rd;

        comm_mkreq(&debug->tx, DPORT_CMD_REQ_FLASH, (uint8*)req,
                rd + DPORT_CMD_FLASH_ARG_SIZE);
        if (comm_txrx(debug) < 0)
        {
            fprintf(stderr, "(E) failed to write flash\n");
            ret = -1;
            goto out;
        }

        if (resp->flash.offset != offset || resp->flash.length != rd)
        {
            fprintf(stderr, "(E) offset/length mismatch, %s\n", __FUNCTION__);
            ret = -1;
            goto out;
        }

        len    -= rd;
        offset += rd;

end:
        /* print progress */
        if ((cycle++ % 8) == 0 || rd == 0 || len == 0)
            debug_print_percent(fsize, len);
        if (rd == 0)
            break;
    }
    printf("\n");
out:
    close(fd);
    return ret;
}

/*
 * write data to target's flash
 */
int debug_cmd_write_flash(struct debug_t *debug, char *data, uint32 offset, uint32 length)
{
    uint32 fbsize;
    union dport_req_t *req; 
    union dport_resp_t *resp; 
    int cycle;
    int n;
    uint32 total;

    if (debug_cmd_get_flash_bufsize(debug, &fbsize) < 0)
        return -1;

    req  = &debug->req;
    resp = (union dport_resp_t *)&debug->rx.buf[2];

    total = length;
    cycle = 0;
    printf("write flash at 0x%08X ... " PERCENT_GAP, offset);
    while (length)
    {
        n = fbsize;
        if (n > length)
            n = length;

        req->flash.offset = offset;
        req->flash.length = n;

        memcpy(req->flash.data, data, n);

        comm_mkreq(&debug->tx, DPORT_CMD_REQ_FLASH, (uint8*)req,
                n + DPORT_CMD_FLASH_ARG_SIZE);
        if (comm_txrx(debug) < 0)
        {
            fprintf(stderr, "(E) failed to write flash\n");
            return -1;
        }

        if (resp->flash.offset != offset || resp->flash.length != n)
        {
            fprintf(stderr, "(E) offset/length mismatch, %s\n", __FUNCTION__);
            return -1;
        }

        length -= n;
        offset += n;
        data   += n;

        /* print progress */
        if ((cycle++ % 8) == 0 || length == 0)
            debug_print_percent(total, length);
    }
    printf("\n");

    return 0;
}

/*
 * send user function call to target
 *
 * RETURN
 *     0 on success, -1 on error
 */
int debug_cmd_userf(struct debug_t *debug, int nfunc)
{
    union dport_req_t *req; 
    int rd, i;

    req = &debug->req;

    req->userf.n = nfunc;

    comm_mkreq(&debug->tx, DPORT_CMD_REQ_USERF, (uint8*)req,  sizeof(struct dport_req_userf_t));
    if (comm_txrx(debug) < 0)
        return -1;

    /* ouput to screen everething that was received from debug port */
    do {
        rd = commdev_recv(debug->commdev, debug->rx.buf, DEBUG_BUF_SIZE, 500);
        if (rd > 0)
        {
            for (i = 0; i < rd; i++)
                printf("%c", debug->rx.buf[i]);
            fflush(stdout);
            continue;
        }
    } while (rd > 0);

    return 0;
}


