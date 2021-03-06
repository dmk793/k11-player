/*
 * Pearson hash algorithm (http://en.wikipedia.org/wiki/Pearson_hashing).
 *
 *     h := 0
 *     for each c in C loop
 *       index := h xor c
 *       h := T[index]
 *     end loop
 *     return h
 *
 * For a given string or chunk of data, Pearson's original algorithm produces
 * only an 8 bit byte or integer, 0-255. But the algorithm makes it extremely
 * easy to generate whatever length of hash is desired. The scheme used above
 * is a very straightforward implementation of the algorithm. As Pearson noted
 * a change to any bit in the string causes his algorithm to create a completely
 * different hash (0-255). In the code below, following every completion of
 * the inner loop, the first byte of the string is incremented by one. x[0]=x[0]+1.
 *
 * Every time that simple change to the first byte of the data is made, a different
 * Pearson hash, h, is generated. xPear16 builds a 16 hex byte hash by concatenating
 * a series of 8-bit Pearson (h) hashes. Instead of producing a value from 0
 * to 255, it generates a value from 0 to 18,446,744,073,709,551,615.
 *
 * Pearson's algorithm can be made to generate hashes of any desired length
 * simply by adding 1 to the first byte of the string, re-computing h for the
 * string, and concatenating the results. Thus the same core logic can be made
 * to generate 32-bit or 128-bit hashes.
 *
 *
 */

#include <types.h>

uint8 T[256] = {
        0x62, 0x06, 0x55, 0x96, 0x24, 0x17, 0x70, 0xa4, 0x87, 0xcf, 0xa9, 0x05, 0x1a, 0x40, 0xa5, 0xdb, // 0x00
        0x3d, 0x14, 0x44, 0x59, 0x82, 0x3f, 0x34, 0x66, 0x18, 0xe5, 0x84, 0xf5, 0x50, 0xd8, 0xc3, 0x73, // 0x01
        0x5a, 0xa8, 0x9c, 0xcb, 0xb1, 0x78, 0x02, 0xbe, 0xbc, 0x07, 0x64, 0xb9, 0xae, 0xf3, 0xa2, 0x0a, // 0x02
        0xed, 0x12, 0xfd, 0xe1, 0x08, 0xd0, 0xac, 0xf4, 0xff, 0x7e, 0x65, 0x4f, 0x91, 0xeb, 0xe4, 0x79, // 0x03
        0x7b, 0xfb, 0x43, 0xfa, 0xa1, 0x00, 0x6b, 0x61, 0xf1, 0x6f, 0xb5, 0x52, 0xf9, 0x21, 0x45, 0x37, // 0x04
        0x3b, 0x99, 0x1d, 0x09, 0xd5, 0xa7, 0x54, 0x5d, 0x1e, 0x2e, 0x5e, 0x4b, 0x97, 0x72, 0x49, 0xde, // 0x05
        0xc5, 0x60, 0xd2, 0x2d, 0x10, 0xe3, 0xf8, 0xca, 0x33, 0x98, 0xfc, 0x7d, 0x51, 0xce, 0xd7, 0xba, // 0x06
        0x27, 0x9e, 0xb2, 0xbb, 0x83, 0x88, 0x01, 0x31, 0x32, 0x11, 0x8d, 0x5b, 0x2f, 0x81, 0x3c, 0x63, // 0x07
        0x9a, 0x23, 0x56, 0xab, 0x69, 0x22, 0x26, 0xc8, 0x93, 0x3a, 0x4d, 0x76, 0xad, 0xf6, 0x4c, 0xfe, // 0x08
        0x85, 0xe8, 0xc4, 0x90, 0xc6, 0x7c, 0x35, 0x04, 0x6c, 0x4a, 0xdf, 0xea, 0x86, 0xe6, 0x9d, 0x8b, // 0x09
        0xbd, 0xcd, 0xc7, 0x80, 0xb0, 0x13, 0xd3, 0xec, 0x7f, 0xc0, 0xe7, 0x46, 0xe9, 0x58, 0x92, 0x2c, // 0x0a
        0xb7, 0xc9, 0x16, 0x53, 0x0d, 0xd6, 0x74, 0x6d, 0x9f, 0x20, 0x5f, 0xe2, 0x8c, 0xdc, 0x39, 0x0c, // 0x0b
        0xdd, 0x1f, 0xd1, 0xb6, 0x8f, 0x5c, 0x95, 0xb8, 0x94, 0x3e, 0x71, 0x41, 0x25, 0x1b, 0x6a, 0xa6, // 0x0c
        0x03, 0x0e, 0xcc, 0x48, 0x15, 0x29, 0x38, 0x42, 0x1c, 0xc1, 0x28, 0xd9, 0x19, 0x36, 0xb3, 0x75, // 0x0d
        0xee, 0x57, 0xf0, 0x9b, 0xb4, 0xaa, 0xf2, 0xd4, 0xbf, 0xa3, 0x4e, 0xda, 0x89, 0xc2, 0xaf, 0x6e, // 0x0e
        0x2b, 0x77, 0xe0, 0x47, 0x7a, 0x8e, 0x2a, 0xa0, 0x68, 0x30, 0xf7, 0x67, 0x0f, 0x0b, 0x8a, 0xef  // 0x0f
    };

/*
 *
 */
uint32 pear32(uint8 *x, int len)
{
    int i, j;
#define HASH_LEN    4
    uint8 hh[HASH_LEN];
    uint8 ch;
    uint8 h;

    ch = x[0]; /* save first byte */
    for (j = 0; j < HASH_LEN; j++)
    {
        /* standard Pearson hash (output is h) */
        h = 0;
        for (i = 0; i < len; i++)
        {
            h = T[h ^ x[i]];
        }
        hh[j] = h;        /* store result */
        x[0]  = x[0] + 1; /* increment first data byte by 1 */
    }
    x[0] = ch; /* restore first byte */

    return (hh[0] | (hh[1] << 8) | (hh[2] << 16) | (hh[3] << 24));
}

