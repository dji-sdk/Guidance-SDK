#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crc32.h"

#define POLYNOMIAL 0x04c11db7L      // Standard CRC-32 ppolynomial
#define BUFFER_LEN       4096L      // Length of buffer

static word32 crc_table[256];       // Table of 8-bit remainders

class crc32
{
public:
	crc32();
};

crc32::crc32()
{
	gen_crc_table();
}

crc32 g_crc32;

void gen_crc_table(void)
{
	register word16 i, j;
	register word32 crc_accum;

	for (i=0;  i<256;  i++)
	{
		crc_accum = ( (word32) i << 24 );
		for ( j = 0;  j < 8;  j++ )
		{
			if ( crc_accum & 0x80000000L )
				crc_accum = (crc_accum << 1) ^ POLYNOMIAL;
			else
				crc_accum = (crc_accum << 1);
		}
		crc_table[i] = crc_accum;
	}
}

word32 update_crc(word32 crc_accum, byte *data_blk_ptr, word32 data_blk_size)
{
	if ( data_blk_size < sizeof(word32) )
	{
		return 0;
	}

	register word32 i, j;

	for (j=0; j<data_blk_size; j++)
	{
		i = ((int) (crc_accum >> 24) ^ *data_blk_ptr++) & 0xFF;
		crc_accum = (crc_accum << 8) ^ crc_table[i];
	}
	crc_accum = ~crc_accum;
	memcpy( data_blk_ptr + data_blk_size - sizeof(word32), &crc_accum, sizeof(word32) );

	return crc_accum;
}
