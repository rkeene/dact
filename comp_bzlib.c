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


#include "dact.h"
#include "comp_bzlib.h"

#ifdef HAVE_LIBBZ2
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <stdio.h>
#ifdef HAVE_BZLIB_H
#include <bzlib.h>
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
uint32_t DC_NUM=9;
uint32_t DC_TYPE=DACT_MOD_TYPE_COMP;
void *DC_ALGO=comp_bzlib_algo;
char *DC_NAME="Bzip2 Compression (MOD)";
#endif

int comp_bzlib_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, unsigned char *out_block, int blk_size, int bufsize) {
	switch(mode) {
		case DACT_MODE_COMPR:
			return(comp_bzlib_compress(prev_block, curr_block, out_block, blk_size, bufsize));
			break; /* Heh */
		case DACT_MODE_DECMP:
			return(comp_bzlib_decompress(prev_block, curr_block, out_block, blk_size, bufsize));
			break;
		default:
			fprintf(stderr, "Unsupported mode: %i\n", mode);
			return(-1);
	}
}

int comp_bzlib_compress(unsigned char *prev_block, unsigned char *curr_block, unsigned char *out_block, int blk_size, int bufsize) {
	unsigned int dest_size=bufsize;
	int retval;

#ifdef HAVE_OLD_BZ2
	retval=bzBuffToBuffCompress((char *) out_block, &dest_size, (char *) curr_block, blk_size, 9, 0, 9);
#else
	retval=BZ2_bzBuffToBuffCompress((char *) out_block, &dest_size, (char *) curr_block, blk_size, 9, 0, 9);
#endif
/* Remove the "BZh9" header. */
	dest_size-=4;
	memmove(out_block, out_block+4, dest_size);

	if (retval!=BZ_OK) return(-1);

	return(dest_size);
}

int comp_bzlib_decompress(unsigned char *prev_block, unsigned char *curr_block, unsigned char *out_block, int blk_size, int bufsize) {
	unsigned int dest_size=bufsize;
	char *tmpbuf;
	int retval;

/* Replant the header. */
	tmpbuf=malloc(blk_size+4);
	memcpy(tmpbuf, "BZh9", 4);
	memcpy(tmpbuf+4, curr_block, blk_size);

#ifdef HAVE_OLD_BZ2
	retval=bzBuffToBuffDecompress((char *) out_block, &dest_size, tmpbuf, blk_size+4, 0, 0);
#else
	retval=BZ2_bzBuffToBuffDecompress((char *) out_block, &dest_size, tmpbuf, blk_size+4, 0, 0);
#endif

	free(tmpbuf);

	if (retval!=BZ_OK) return(0);
	return(dest_size);
}
#endif
