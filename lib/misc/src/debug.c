/*
 *
 */
#include <stdarg.h>
#include <string.h>
#include "debug.h"

int putChar(int);

#define NEW_LINE "\r\n"
#define TAB_STR  "    "

/*
 * RETURN
 *     if "dst" not NULL, return "dst + 1"
 */
char* udebug_char(char *dst, int ch)
{
    if (dst)
    {
        *dst = ch;
        return (dst + 1);
    }

    putChar(ch);
    return NULL;
}

/*
 *
 */
char *udebug_str(char *dst, char *str)
{
    while (str != NULL && *str)
        dst = udebug_char(dst, *str++);

    return dst;
}

/*
 * NOTE result string not null terminated
 */
char *udebug_hex(char *dst, void *p, uint32 size)
{
    int32 i;
    uint8 *pdata;
    uint8 data;

    pdata = (uint8*)p;

    for (i = size-1; i >= 0; i--)
    {
        data = pdata[i] >> 4;

        if (data >= 10)
            dst = udebug_char(dst, data + 'A' - 10);
        else
            dst = udebug_char(dst, data + '0');

        data = pdata[i] & 0x0f;

        if (data >= 10)
            dst = udebug_char(dst, data + 'A' - 10);
        else
            dst = udebug_char(dst, data + '0');
    }

    return dst;
}

/*
 *
 */
char *udebug_bin(char *dst, void *p, uint32 size)
{
    int32 i, j;
    uint8 *pdata;
    uint8 data;

    pdata = (uint8*)p;

    for (i = size-1; i >= 0; i--)
    {
        data = pdata[i];
        for (j = 7; j >= 0; j--)
        {
            if (data & (1 << j))
                dst = udebug_char(dst, '1');
            else
                dst = udebug_char(dst, '0');
        }
        if (i != 0)
            dst = udebug_char(dst, '.');
    }

    return dst;
}

/*
 *
 */
char *udebug_dec(char *dst, void *p, uint32 size, int *slen)
{
    int c;
    uint32 start;
    uint32 num;
    uint8 n, nn;

    if (size == 4) {
        start = 1000000000;
        num = *((uint32*)p);
    } else if (size == 2) {
        start = 10000;
        num = *((uint16*)p);
    } else if (size == 1) {
        start = 100;
        num = *((uint8*)p);
    } else
        return dst;

    nn = 0;
    c  = 0;
    while (start > 1)
    {
        n = num / start;
        if (n > 0 || nn)
        {
            nn = 1;
            dst = udebug_char(dst, n + '0');
            c++;
        }

        num = num % start;
        start /= 10;
    }
    n = num % 10;
    dst = udebug_char(dst, n + '0');
    c++;

    if (slen)
        *slen = c;
    return dst;
}

/*
 *
 */
static char *udebug_phead(char *dst, int size)
{
    int i;
    /* print header */
    {
        udebug_str(dst, NEW_LINE);
        if (size < 0x100)
            dst = udebug_str(dst, "..");
        else if (size < 0x10000)
            dst = udebug_str(dst, "....");
        else if (size < 0xffffffff)
            dst = udebug_str(dst, ".........");

        dst = udebug_str(dst, "..");
        for (i = 0; i < 16; i++)
        {
            if (i && ((i % 8) == 0))
                dst = udebug_char(dst, '.');
            dst = udebug_hex(dst, &i, 1);
            dst = udebug_char(dst, '.');
        }
    }

    return dst;
}

/*
 *
 * FORMAT
 *     s    output string
 *     n    output newline (CR + LF)
 *     c    output symbol
 *     Np   output value of pointer (N 1 to infinity)
 *     Nx   output number in hex format (N is optional count of bytes
 *          in number in range from 1 to 4)
 *     Nd   output number in decimal format
 *     l    length for buffer to output
 *     g    size of padding of string or decimal number
 *     b    output buffer (length specified with previou s'l' argument)
 *     ' '  do nothing
 *     _    output space
 *     t    output tab
 *     *    treat following character as symbol (output directly)
 *     <    switch direct output on
 *     >    switch direct output off
 *
 * NOTE if "dst" not NULL, result string null terminated
 *
 */
void udprint(char *dst, char *fmt, ...)
{
    va_list ap;
    int flen;
    int c;
    char *s;
    void *p;
    int num, wasnum;
    unsigned int dec;
    int direct, direct2;
    int pad;

#define DEFAULT_FLEN    4
    flen = DEFAULT_FLEN;
    va_start(ap, fmt);
    num = 0;
    dec = 0;
    direct  = 0;
    direct2 = 0;
    pad = 0;
    while (*fmt)
    {
        wasnum = 0;
        if (*fmt != '<' && *fmt != '>' && (direct || direct2))
        {
            direct = 0;
            dst = udebug_char(dst, *fmt++);
            continue;
        }

        switch (*fmt)
        {
            case '*':
                direct = 1;
                break;
            case '<':
                direct2 = 1;
                break;
            case '>':
                direct2 = 0;
                break;
            case 'c':
                c = va_arg(ap, int);
                dst = udebug_char(dst, c);
                break;
            case 'p':
                p = va_arg(ap, void*);
                if (p)
                    dst = udebug_hex(dst, p, flen);
                flen = DEFAULT_FLEN;
                break;
            case 'l':
            case 'g':
                pad = va_arg(ap, int);
                break;
            case 'b':
                {
                    int i;
                    uint8 *buf;
                    uint32 offset;

                    buf = va_arg(ap, void*);
                    offset = 0;
                    for (i = 0; i < pad; i++)
                    {
                        if ((i % 256) == 0)
                            dst = udebug_phead(dst, pad);
                        if ((i % 16) == 0)
                        {
                            dst = udebug_str(dst, "."NEW_LINE);
                            if (pad < 0x100)
                                dst = udebug_hex(dst, &offset, 1);
                            else if (pad < 0x10000)
                                dst = udebug_hex(dst, &offset, 2);
                            else if (pad < 0xffffffff)
                                dst = udebug_hex(dst, &offset, 4);
                            dst = udebug_str(dst, "  ");
                            offset += 16;
                        } else {
                            if ((i % 8) == 0)
                                dst = udebug_char(dst, ' ');
                        }

                        dst = udebug_hex(dst, buf++, 1);
                        dst = udebug_char(dst, ' ');
                    }
                    dst = udebug_str(dst, NEW_LINE);
                    pad = 0;
                }
                break;
            case 'x':
            case 'd':
                if (*fmt == 'x')
                {
                    dec = va_arg(ap, int);
                    dst = udebug_hex(dst, &dec, flen);
                } else {
                    dec = va_arg(ap, int);
                    dst = udebug_dec(dst, &dec, flen, &num); /* NOTE spoil "num" variable */
                    if (pad)
                    {
                        int i;

                        if (pad > num)
                            pad -= num;
                        else
                            pad = 0;
                        for (i = 0; i < pad; i++)
                            dst = udebug_char(dst, ' ');
                        pad = 0;
                    }
                }
                flen = DEFAULT_FLEN;
                break;
            case 's':
                s = va_arg(ap, char*);
                dst = udebug_str(dst, s);
                if (pad)
                {
                    int i;

                    dec = strlen(s); /* NOTE spoil variable */
                    if (pad > dec)
                        pad -= dec;
                    else
                        pad = 0;
                    for (i = 0; i < pad; i++)
                        dst = udebug_char(dst, ' ');
                    pad = 0;
                }
                break;
            case 'n':
                dst = udebug_str(dst, NEW_LINE);
                break;
            case '_':
                dst = udebug_char(dst, ' ');
                break;
            case 't':
                dst = udebug_str(dst, TAB_STR);
                break;
            default:
                /* assume format is number or unknown */
                if (*fmt >= '0' && *fmt <= '9')
                {
                    flen = (flen * ((num++) * 10) + (*fmt - '0'));
                    wasnum = 1;
                }
        }
        if (!wasnum)
            num = 0;
        fmt++;
    }
    va_end(ap);

    if (dst)
        *dst = 0; /* NOTE null-terminate */
}


