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
#include "comp_lzooy.h"
#ifndef comp_lzooy_algo
#include <lzoconf.h>
#include <lzo1y.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif


/*
	mode 		- DACT_MODE_COMPR or DACT_MODE_DECMP
			    Determine whether to compress or decompress.
	prev_block	- Previous (uncompressed) block.
	curr_block	- The data to be compressed.
	out_block	- Where to put data after compression.
	blk_size	- Size of prev_block and curr_block.
*/

#if defined(AS_MODULE) && defined(USE_MODULES)
#include "module.h"
uint32_t DC_NUM=14;
uint32_t DC_TYPE=DACT_MOD_TYPE_COMP;
void *DC_ALGO=comp_lzooy_algo;
char *DC_NAME="LZO-1y Compression (MOD)";
/*
char *DC_SIGN="RGlnaXRhbFNpZ25hdHVyZQ==";
char *DC_URL_GET="http://www.rkeene.org/projects/compression/dact/@@OSNM@@-@@ARCH@@/comp_rle.dll";
char *DC_URL_VER="http://www.rkeene.org/projects/compression/dact/@@OSNM@@-@@ARCH@@/comp_rle.ver";
uint32_t DC_VER=0x00080d;
uint32_t DC_REQUIRE=DACT_MOD_REQ_ATLEAST|(0x00080d);
*/
#endif

int comp_lzooy_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	switch(mode) {
		case DACT_MODE_COMPR:
			return(comp_lzooy_compress(prev_block, curr_block, out_block, blk_size, bufsize));
			break; /* Heh */
		case DACT_MODE_DECMP:
			return(comp_lzooy_decompress(prev_block, curr_block, out_block, blk_size, bufsize));
			break;
		default:
			printf("Unsupported mode: %i\n", mode);
			return(-1);
	}
}

int comp_lzooy_compress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	int retsize=0;
	char *wrkmem;

	if ((wrkmem=malloc(LZO1Y_999_MEM_COMPRESS))==NULL) return(-1);
	if (lzo_init()!=LZO_E_OK) return(-1);
	lzo1y_999_compress(curr_block, blk_size, out_block, &retsize, wrkmem);
	free(wrkmem);
	return(retsize);
}

int comp_lzooy_decompress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	int retsize=0, retval;

	if (lzo_init()!=LZO_E_OK) return(-1);
	retval=lzo1y_decompress(curr_block, blk_size, out_block, &retsize, NULL);
	if (retval!=LZO_E_OK) return(-1);
	return(retsize);
}
#endif
