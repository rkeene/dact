/*
 * Copyright (C) 2001  Roy Keene
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
	Use "Range Encoding."

  http://www.compressconsult.com/ or http://eiunix.tuwien.ac.at/~michael
  michael@compressconsult.com        michael@eiunix.tuwien.ac.at

*/


#include "dact.h"
#include "comp_range.h"
#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"

#include "comp_range/rangecod.h"
#include "comp_range/port.h"


char *tmp_block;

int count_i=0;

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
uint32_t DC_NUM=7;
uint32_t DC_TYPE=DACT_MOD_TYPE_COMP;
void *DC_ALGO=comp_range_algo;
char *DC_NAME="Range Encoding (MOD)";
#endif

int comp_range_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size) {
	switch(mode) {
		case DACT_MODE_COMPR:
			return(comp_range_compress(prev_block,curr_block,out_block,blk_size));
			break; /* Heh */
		case DACT_MODE_DECMP:
			return(comp_range_decompress(prev_block,curr_block,out_block,blk_size));
			break;
		default:
			printf("Unsupported mode: %i\n", mode);
			return(-1);
	}
}

int comp_range_compress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size) {
	unsigned int ch;
	freq counts[257];
	rangecoder rc;
	int i,x;

	if ((tmp_block=malloc(DACT_BLK_SIZE))==NULL) {
		return(-1);
	}


	count_i=0;
	for (x=0;x<DACT_BLK_SIZE;x++) tmp_block[x]=0;
	byte_buffer_purge();
	byte_buffer_write(curr_block, blk_size);

	start_encoding(&rc,0,0);
	encode_freq(&rc,1,1,2);

	for (x=0;x<257;x++) counts[x]=0;
	for (x=0;x<blk_size;x++) counts[curr_block[x]]++;


	for (x=0;x<256;x++) {
		encode_short(&rc,counts[x]);
/*		printf("={ 0x%02x, 0x%04x }\n",x,counts[x]); */
	}

	counts[256]=blk_size;
	for (x=256; x; x--) counts[x-1]=counts[x]-counts[x-1];

	for (i=0;i<blk_size;i++) {
		ch=curr_block[i];
/*		printf("i=%5i ch=0x%02x -- counts[ch+1]=0x%02x counts[ch]=0x%02x\n",i,ch,counts[ch+1],counts[ch]); */
		encode_freq(&rc,counts[ch+1]-counts[ch],counts[ch],counts[256]);
	}

	encode_freq(&rc,1,0,2);

	done_encoding(&rc);


	if (count_i>blk_size) return(-1);
	for (i=0;i<count_i;i++) {
		out_block[i]=tmp_block[i];
	}

	free(tmp_block);
	return(count_i);
}

int comp_range_decompress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size) {
	int i,x;
	int blocksize;
	freq counts[257], tmp, cf, symbol;
	rangecoder rc;

	tmp_block=malloc(DACT_BLK_SIZE);

	byte_buffer_purge();
	byte_buffer_write(curr_block, blk_size);
	count_i=0;
	for (x=0;x<DACT_BLK_SIZE;x++) tmp_block[x]=0;

	start_decoding(&rc);
	cf = decode_culfreq(&rc,2);
	decode_update(&rc,1,1,2);

	for (x=0; x<256; x++) counts[x]=decode_short(&rc);

	blocksize=0;
	for (x=0; x<256; x++) {
		tmp=counts[x];
		counts[x]=blocksize;
		blocksize+=tmp;
	}
	counts[256]=blocksize;

	for (i=0; i<blocksize; i++) {
		cf=decode_culfreq(&rc,counts[256]);
		for (symbol=0; counts[symbol+1]<=cf; symbol++) { /**/; }
		decode_update(&rc, counts[symbol+1]-counts[symbol],counts[symbol],counts[256]);
	
		incoming_data(symbol);
	}

	done_decoding(&rc);

// Why am I not memcpy()'ing?
	for (i=0;i<count_i;i++) out_block[i]=tmp_block[i];

	free(tmp_block);
	return(count_i);
}

void incoming_data(unsigned char value) {
	if (count_i<=(BYTE_BUFF_SIZE))
		tmp_block[count_i++]=value;
}
