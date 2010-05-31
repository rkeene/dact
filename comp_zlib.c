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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      email: dact@rkeene.org
 */


/*
	Example block compression routine for interfacing with DACT.

	Uses zlib.
*/


#include "dact.h"
#include "comp_zlib.h"

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
uint32_t DC_NUM=4;
uint32_t DC_TYPE=DACT_MOD_TYPE_COMP;
void *DC_ALGO=comp_zlib_algo;
char *DC_NAME="Zlib Compression (MOD)";
#endif

int comp_zlib_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, unsigned char *out_block, int blk_size, int bufsize) {
	switch(mode) {
		case DACT_MODE_COMPR:
			return(comp_zlib_compress(prev_block, curr_block, out_block, blk_size, bufsize));
			break; /* Heh */
		case DACT_MODE_DECMP:
			return(comp_zlib_decompress(prev_block, curr_block, out_block, blk_size, bufsize));
			break;
		default:
			printf("Unsupported mode: %i\n", mode);
			return(-1);
	}
}

int comp_zlib_compress(unsigned char *prev_block, unsigned char *curr_block, unsigned char *out_block, int blk_size, int bufsize) {
	unsigned long dest_size;
	int retval;
	dest_size=(int) ((float) (blk_size*1.01)+13);

#ifdef HAVE_COMPRESS2
	retval=compress2(out_block, &dest_size, curr_block, blk_size, Z_BEST_COMPRESSION);
#else
	retval=compress(out_block, &dest_size, curr_block, blk_size);
#endif
	if (retval!=Z_OK) return(-1);
/* Remove the 120,218 header */
	dest_size-=2;
	if (out_block[0]!=120 || out_block[1]!=218) {
		PRINTERR("Error!  Invalid headers, output will most likely be unusable.");
		return(-1);
	}
	memmove(out_block, out_block+2, dest_size);

	return(dest_size);
}

int comp_zlib_decompress(unsigned char *prev_block, unsigned char *curr_block, unsigned char *out_block, int blk_size, int bufsize) {
	unsigned long dest_size;
	int real_blksize=blk_size;
	unsigned char *tmpbuf;
	int retval;

/* Replant the header. */
	if (curr_block[0]==120 && curr_block[1]==218) {
		tmpbuf=curr_block;
	} else {
		tmpbuf=malloc(real_blksize+2);
		tmpbuf[0]=120;
		tmpbuf[1]=218;
		memcpy(tmpbuf+2, curr_block, real_blksize);
		real_blksize+=2;
	}

	dest_size=bufsize;
	retval=uncompress(out_block, &dest_size, tmpbuf, real_blksize);

	if (tmpbuf!=curr_block) free(tmpbuf);

	if (retval!=Z_OK) return(0);

	return(dest_size);
}
#endif
