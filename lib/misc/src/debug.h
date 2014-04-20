#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <types.h>

#define USE_DEBUG

#define ERR_PREFIX    "(E) "
#define WARN_PREFIX   "(W) "

#ifdef USE_DEBUG
    #define debug_char(ch) udebug_char(NULL, ch)
    #define debug_str(st)  udebug_str(NULL, st)
    #define dprint(fmt, ...) udprint(NULL, fmt, __VA_ARGS__)
#else
    #define debug_char(ch)
    #define debug_str(st)
    #define dprint(fmt, ...)
#endif

typedef int (*debug_cb_func)(int);

char *udebug_char(char *dst, int ch);
char *udebug_str(char *dst, char *st);
char *udebug_hex(char *dst, void *p, uint32 size);
char *udebug_bin(char *dst, void *p, uint32 size);
char *udebug_dec(char *dst, void *p, uint32 size, int *slen);
void udprint(char *dst, char *fmt, ...);

#define sprint(dst, fmt, ...) udprint(dst, fmt, __VA_ARGS__)

#define DEBUG_IMSG(msg) \
            DPRINT("s*:_sn", __FUNCTION__, msg)
#define DEBUG_IMSGF(msg, fmt, ...) \
        {                                                      \
            DPRINT("s*:_s_", __FUNCTION__, msg); \
            DPRINT(fmt, __VA_ARGS__);                          \
        }

#define DEBUG_WMSG(msg) \
            DPRINT("ss*:_sn", WARN_PREFIX, __FUNCTION__, msg)
#define DEBUG_WMSGF(msg, fmt, ...)                             \
        {                                                      \
            DPRINT("ss*:_s_", WARN_PREFIX, __FUNCTION__, msg); \
            DPRINT(fmt, __VA_ARGS__);                          \
        }

#define DEBUG_EMSG(msg) \
            DPRINT("ss*:_sn", ERR_PREFIX, __FUNCTION__, msg)
#define DEBUG_EMSGF(msg, fmt, ...)                            \
        {                                                     \
            DPRINT("ss*:_s_", ERR_PREFIX, __FUNCTION__, msg); \
            DPRINT(fmt, __VA_ARGS__);                         \
        }
#endif

