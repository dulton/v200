#ifndef CRC32_H
#define CRC32_H

/* $Id: crc32.h,v 1.6 2005/11/07 11:15:09 gleixner Exp $ */

#include <stdint.h>

extern const unsigned long  crc32_table[256];

/* Return a 32-bit CRC of the contents of the buffer. */
unsigned long 
crc32(unsigned long  rawval, const void *ss, int len);

#endif
