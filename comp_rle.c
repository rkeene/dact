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
#include "comp_rle.h"
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
uint32_t DC_NUM=1;
uint32_t DC_TYPE=DACT_MOD_TYPE_COMP;
void *DC_ALGO=comp_rle_algo;
char *DC_NAME="RLE Compression (MOD)";
char *DC_SIGN="RGlnaXRhbFNpZ25hdHVyZQ==";
char *DC_URL_GET="http://www.rkeene.org/projects/compression/dact/@@OSNM@@-@@ARCH@@/comp_rle.dll";
char *DC_URL_VER="http://www.rkeene.org/projects/compression/dact/@@OSNM@@-@@ARCH@@/comp_rle.ver";
uint32_t DC_VER=0x00080d;
uint32_t DC_REQUIRE=DACT_MOD_REQ_ATLEAST|(0x00080d);
#endif

int comp_rle_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	switch(mode) {
		case DACT_MODE_COMPR:
			return(comp_rle_compress(prev_block, curr_block, out_block, blk_size, bufsize));
			break; /* Heh */
		case DACT_MODE_DECMP:
			return(comp_rle_decompress(prev_block, curr_block, out_block, blk_size, bufsize));
			break;
		default:
			printf("Unsupported mode: %i\n", mode);
			return(-1);
	}
}

int comp_rle_compress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	int i,x=0,m;
	unsigned char sentinel=0xff;
	unsigned char currchar=0, prevchar;
	unsigned char charcnt=0;
	unsigned int lowestcnt=0xffff;
	unsigned int count[256];

	for (i=0;i<256;i++) count[i]=0;

	for (i=0;i<blk_size;i++) count[curr_block[i]]++;

	for (i=0;i<256;i++) {
		if (count[i]<lowestcnt) {
			sentinel=i;
			lowestcnt=count[i];
		}
	}

	out_block[x++]=sentinel;

	prevchar=curr_block[0];
	for (i=0;i<(blk_size+1);i++) {
		if (i!=blk_size) { currchar=curr_block[i]; } else { currchar++; }
		if (prevchar!=currchar || charcnt==255 || i>=(blk_size)) {
			if (charcnt<=2 && prevchar!=sentinel) {
				for (m=0;m<charcnt;m++) out_block[x++]=prevchar;
			} else {
				out_block[x++]=sentinel;
				out_block[x++]=prevchar;
				out_block[x++]=charcnt;
			}
			charcnt=0;
		}
		charcnt++;
		prevchar=currchar;
	}
	return(x);
}

int comp_rle_decompress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	int i,x=0,m;
	unsigned char sentinel=curr_block[0];
	unsigned char currchar;
	unsigned char charcnt;

	for (i=1;i<blk_size;i++) {
		currchar=curr_block[i];
		if (currchar==sentinel) {
			currchar=curr_block[++i];
			charcnt=curr_block[++i];
			if ((x+charcnt)>bufsize) {
				printf("Error in RLE compression!\n");
				return(0);
			}
			for (m=0;m<charcnt;m++) out_block[x++]=currchar;
		} else {
			out_block[x++]=currchar;
		}
	}
	return(x);
}
