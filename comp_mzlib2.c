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


/*
	Compress by gathering bytes that are most often placed near each other
	and rearranging the ASCII Charectar Set to make those bytes have
	similar values.

*/


#include "dact.h"
#include "comp_mzlib2.h"
#ifndef comp_mzlib2_algo

#ifdef HAVE_LIBZ
#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif
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
#include "sort.h"
#define SQRD_256 65536


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
uint32_t DACT_MOD_NUM=8;
uint32_t DACT_MOD_TYPE=DACT_MOD_TYPE_COMP;
void *DACT_MOD_ALGO=comp_mzlib2_algo;
char *DACT_MOD_NAME="Second Modified Zlib Compression (MOD)";
#endif

#ifdef DEBUG
#if DEBUG>=2
#define MAKE_MZLIB2 1
#endif
#endif

#ifdef MAKE_MZLIB2
int comp_mzlib2_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, unsigned char *out_block, int blk_size, int bufsize) {
	switch(mode) {
		case DACT_MODE_COMPR:
			return(comp_mzlib2_compress(prev_block, curr_block, out_block, blk_size, bufsize));
			break; /* Heh */
		case DACT_MODE_DECMP:
			return(comp_mzlib2_decompress(prev_block, curr_block, out_block, blk_size, bufsize));
			break;
		default:
			printf("Unsupported mode: %i\n", mode);
			return(-1);
	}
}
#else
int comp_mzlib2_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, unsigned char *out_block, int blk_size, int bufsize) {
	return(-1);
}
#endif

int comp_mzlib2_compress(unsigned char *prev_block, unsigned char *curr_block, unsigned char *out_block, int blk_size, int bufsize) {
	unsigned long dest_size;
	unsigned char *tmp_block;
	uint32_t freq[SQRD_256];
	uint16_t lookup_table[SQRD_256], curr_val, repl_val;
	int i, cnt, m=2, retval, low_byte=0, x;

	memset(freq,0,sizeof(freq));

	for (i=0;i<blk_size;i+=2) {
		x=(curr_block[i]<<8)|(curr_block[i+1]);
		freq[x]++;
		if (freq[x]==0xffff) return(-1);
	}

	int_sort_fast(freq, SQRD_256, 0);

	for (i=0;i<(SQRD_256);i++) {
		if ((freq[i]&0xffff)!=0) {
			out_block[m++]=freq[i]>>24;
			out_block[m++]=(freq[i]>>16)&0xff;
			if (m>=(blk_size*2)) {
				return(-1);
			}
			lookup_table[freq[i]>>16]=i;
			PRINT_LINE; fprintf(stderr, "%04x: 0x%02x and 0x%02x are near each other %i times\n",i,(freq[i]>>24),(freq[i]>>16)&0xff,freq[i]&0xffff);
		} else {
			break;
		}
	}

	cnt=(m-2);
	out_block[0]=((m-2)>>8);
	out_block[1]=((m-2)&0xff);
	if ((m-2)<0x200) low_byte=1;

	for (i=0;i<blk_size;i+=2) {
		curr_val=((curr_block[i]<<8)| curr_block[i+1]);
		repl_val=lookup_table[curr_val];
		if (!low_byte) 
			out_block[m++]=repl_val>>8;
		out_block[m++]=repl_val&0xff;
SPOTVAR_NUM(m);
		if (m>=(blk_size*2)) return(-1);
	}

	dest_size=(int) ((float) ((m*1.02)+15));
SPOTVAR_NUM(dest_size);
	if ((tmp_block=malloc(dest_size))==NULL) {
		return(-1);
	}
        memcpy(tmp_block,out_block,m);
/*
	for (i=0;i<m;i++) {
		printf("%02x ", tmp_block[i]);
		if (i==(cnt+2)) printf("-- ");
		if (((i+1)%25)==0) printf("\n");
	}
	printf("\n\n");
*/

/*
	for (i=0;i<blk_size;i++) printf("%02x ", curr_block[i]);
	printf("\n");
*/

	retval=compress(out_block, &dest_size, tmp_block, m);
	if (retval!=Z_OK) {
		return(blk_size*2);
	}

	free(tmp_block);
	return(dest_size);
}

int comp_mzlib2_decompress(unsigned char *prev_block, unsigned char *curr_block, unsigned char *out_block, int blk_size, int bufsize) {
	uint16_t lookup_table[SQRD_256], curr_val, repl_val;
	unsigned long dest_size=(blk_size*2);
	unsigned char *tmp_block;
	unsigned int m=0, hdrsize;
	int retval, i, low_byte=0;

	tmp_block=malloc(dest_size);
	retval=uncompress(tmp_block,&dest_size,curr_block,blk_size);

	if (retval!=Z_OK) return(0);

	hdrsize=((unsigned int) (((tmp_block[0]<<8)|tmp_block[1])))+2;

	if ((hdrsize-2)<0x200) low_byte=1;

	for (i=2;i<hdrsize;i+=2) {
		lookup_table[(i-2)/2]=((tmp_block[i]<<8)|tmp_block[i+1]);
	}

	for (i=hdrsize;i<(dest_size);i+=(2-low_byte)) {
		if (low_byte) {
			curr_val=tmp_block[i];
		} else {
			curr_val=((tmp_block[i]<<8)|tmp_block[i+1]);
		}
		repl_val=lookup_table[curr_val];
		out_block[m++]=repl_val>>8;
		out_block[m++]=repl_val&0xff;
	}

/*
	for (i=0;i<m;i++) printf("%02x ", out_block[i]);
	printf("\n");
*/

	free(tmp_block);
	return(m);
}
#endif

#endif
