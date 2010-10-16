/*
 * Copyright (C) 2001, 2002, and 2003  Roy Keene
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *      email: dact@rkeene.org
 */


#include "dact.h"
#include "comp_mzlib.h"

#ifdef HAVE_LIBZ
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif


/*
	mode 		- DACT_MODE_COMPR or DACT_MODE_DECMP
			    Determine whether to compress or decompress.
	prev_block	- Previous (uncompressed) block.
	curr_block	- The data to be compressed.
	out_block	- Where to put data after compression.
	blk_size	- Size of prev_block and curr_block.
*/

#if defined(USE_MODULES) && defined(AS_MODULE)
#include "module.h"
uint32_t DC_NUM=5;
uint32_t DC_TYPE=DACT_MOD_TYPE_COMP;
void *DC_ALGO=comp_mzlib_algo;
char *DC_NAME="Modifed Zlib Compression (MOD)";
#endif

int comp_mzlib_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, unsigned char *out_block, int blk_size, int bufsize) {
	switch(mode) {
		case DACT_MODE_COMPR:
			return(comp_mzlib_compress(prev_block, curr_block, out_block, blk_size, bufsize));
			break; /* Heh */
		case DACT_MODE_DECMP:
			return(comp_mzlib_decompress(prev_block, curr_block, out_block, blk_size, bufsize));
			break;
		default:
			printf("Unsupported mode: %i\n", mode);
			return(-1);
	}
}

int comp_mzlib_compress(unsigned char *prev_block, unsigned char *curr_block, unsigned char *out_block, int blk_size, int bufsize) {
	unsigned long dest_size;
	unsigned char *tmp_block;
	int retval;
	int i;

	dest_size=(int) ((float) (blk_size*1.02)+15);

	if ((tmp_block=malloc(dest_size))==NULL) {
		return(-1);
	}
	memcpy(tmp_block,curr_block,blk_size);

	for (i=0;i<blk_size;i++) {
/*
		tmp_block[i]=( \
			(  (tmp_block[i]&0x80) >> 7 ) + \
			(  (tmp_block[i]&0x20) >> 4 ) + \
			(  (tmp_block[i]&0x08) >> 1 ) + \
			(  (tmp_block[i]&0x02) << 2 ) + \
			(  (tmp_block[i]&0x40) >> 2 ) + \
			(  (tmp_block[i]&0x10) << 1 ) + \
			(  (tmp_block[i]&0x04) << 4 ) + \
			(  (tmp_block[i]&0x01) << 7 ) \
		);
*/
		tmp_block[i]=( ((tmp_block[i]&0xf0)>>4) + (  (tmp_block[i]&0x0f)  <<4) );
	}
#ifdef HAVE_COMPRESS2
	retval=compress2(out_block, &dest_size, tmp_block, blk_size, Z_BEST_COMPRESSION);
#else
	retval=compress(out_block, &dest_size, tmp_block, blk_size);
#endif
	if (retval!=Z_OK) {
		return(-1);
	}
	free(tmp_block);
	return(dest_size);
}

int comp_mzlib_decompress(unsigned char *prev_block, unsigned char *curr_block, unsigned char *out_block, int blk_size, int bufsize) {
	unsigned long dest_size=(blk_size*2);
	int retval;
	int i;

	retval=uncompress(out_block,&dest_size,curr_block,blk_size);

	if (retval!=Z_OK) {
		return(0);
	}

	for (i=0;i<dest_size;i++) {
		out_block[i]=( ( (out_block[i]&0xf0)>>4) + ((out_block[i]&0x0f)<<4) );
	}

	return(dest_size);
}
#endif
