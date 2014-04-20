#ifndef TYPES_H
#define TYPES_H

typedef unsigned char  uint8;
typedef signed   char  int8;
typedef unsigned short uint16;
typedef signed   short int16;
typedef unsigned int   uint32;
typedef signed   int   int32;
typedef unsigned long long int uint64;
typedef signed   long long int int64;

#define BASE_TYPE int32

#define BASE_TYPE_MAX 0xffffffff

#define MIN_INT16  (-32768)
#define MAX_INT16  (32767)

#define TRUE  1
#define FALSE 0

#ifndef NULL
#define NULL ((void*)0)
#endif

#endif

