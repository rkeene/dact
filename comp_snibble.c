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


	Seminibble Compression (Actually, a 2bit Huffman Implementation)

*/


#include "dact.h"
#include "comp_snibble.h"
#include "buffer.h"
#include "sort.h"
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
uint32_t DC_NUM=6;
uint32_t DC_TYPE=DACT_MOD_TYPE_COMP;
void *DC_ALGO=comp_snibble_algo;
char *DC_NAME="Seminibble Encoding (MOD)";
#endif

int comp_snibble_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	switch(mode) {
		case DACT_MODE_COMPR:
			return(comp_snibble_compress(prev_block, curr_block, out_block, blk_size, bufsize));
			break; /* Heh */
		case DACT_MODE_DECMP:
			return(comp_snibble_decompress(prev_block, curr_block, out_block, blk_size, bufsize));
			break;
		default:
			printf("Unsupported mode: %i\n", mode);
			return(-1);
	}
}

int comp_snibble_compress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	const unsigned char hash_table[4]={0, 2, 6, 7};
	const unsigned char hash_len[8]=  {1, 0, 2, 0, 0, 0, 3, 3};
	unsigned char lookup_table[4]={0, 0, 0, 0};
	uint32_t freq[4]={0, 0, 0, 0};
	char *curr_cpy;
	int i,x,m,g=0;

	if ((curr_cpy=malloc(blk_size))==NULL) {
		return(-1);
	}
	memcpy(curr_cpy,curr_block,blk_size);

	bit_buffer_purge();

	for (i=0;i<blk_size;i++) {
		freq[(int) ((curr_cpy[i]&0xc0)>>6)]++;
		freq[(int) ((curr_cpy[i]&0x30)>>4)]++;
		freq[(int) ((curr_cpy[i]&0x0c)>>2)]++;
		freq[(int) ((curr_cpy[i]&0x03))]++;
	}

	int_sort(freq,4,1);

	for (i=0;i<4;i++) lookup_table[freq[i]]=hash_table[i];

	bit_buffer_write(freq[0],2);
	bit_buffer_write(freq[1],2);
	bit_buffer_write(freq[2],2);

/*
	outsize=((int) ((((((float) (freq[0]&0xffff))/2)+(freq[1]&0xffff)+(((float) (freq[2]&0xffff)+(freq[3]&0xffff))*1.5))/4.0)+0.99));

*/
	for (i=0;i<blk_size;i++) {
		for (x=0;x<4;x++) {
			m=((curr_cpy[i]&(3<<(x*2)))>>( x*2 ))  ;
			bit_buffer_write(lookup_table[m],hash_len[lookup_table[m]]);
			while (bit_buffer_size()>=8) out_block[g++]=((unsigned char) (bit_buffer_read(8)));
		}
	}
	i=bit_buffer_size();
	if (i!=0) out_block[g++]=bit_buffer_read(i)<<(8-i);

	free(curr_cpy);

	return(g);

}

int comp_snibble_decompress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size, int bufsize) {
	const unsigned char lookup_table[8]={0, 0, 1, 0, 0, 0, 2, 3};
	unsigned int freq[4];
	unsigned int x, m=0;
	unsigned int i, cnt=0, f=0, j=0;
	int32_t g=0;

	freq[0]=(curr_block[0]&0xc0)>>6; /* 0 */
	freq[1]=(curr_block[0]&0x30)>>4; /* 2 */
	freq[2]=(curr_block[0]&0x0c)>>2; /* 6 */
	for (i=0;i<4;i++) {
		if (freq[0]!=i && freq[1]!=i && freq[2]!=i) freq[3]=i;
	}
	out_block[0]=0;

	bit_buffer_purge();

	bit_buffer_write(curr_block[g++]&0x03,2);

	while (1) {
		while (((bit_buffer_size()+8)<=16) && g<=blk_size) bit_buffer_write(curr_block[g++],8);
		x=bit_buffer_read(1);
		m=(m<<1)+x;
		cnt++;
		if (x==0 || cnt==3) {
			out_block[f]|=(freq[lookup_table[m]]<<( j )) ;
			j+=2;
/* Obsficated comes to mind ... */
			if (j==8) out_block[++f]=(j=0);
			m=0;
			cnt=0;
		}
		if (bit_buffer_size()==0 || f==bufsize) break;
	}

	return(f);
}
