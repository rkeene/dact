/*
 * Copyright (C) 2000  Roy Keene
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
 *      email: rkeene@rkeene.org
 */


/*
	Classic 8-Bit Huffman compression algorithm
	Colin Fowler Jun 2001

	NOTES / TODO
	------------

	Is NOT endian safe - can't exchange data between machines with 
	different endianess (read and write header of ints with memcpy)
	(use ntohl and htonl for a quick fix?)

	Header is 1k - Fix this. I'll rarely get 4gigabytes of the same char 
	but we have to allow for that. Will be easy to cut it down to 512b
	using shorts if a block contains no char with a count above 64k and
	down further to 256b is a block contains no char with a count above 255
	Flag this in the first byte of the header.

	It's SLOW SLOW SLOW and SLOW. Make it faster.
		Look into using lookup tables for decoding.
		Gprof the code.
			most likely optimise the huff_write_out() and the
			huff_bit_write_dict()
						
	Reorder the functions so the code is more readable

	Check for gcc'isms and other stuff another compiler will bork on


Set your tabs to 8 Spaces to make this readable			
*/


#include "dact.h"
#include "comp_huff.h"
#include "buffer.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


huff_node *new_huff_node(unsigned int count, huff_node *left, huff_node *right, unsigned char value)
{
huff_node	*retval;

	retval = (huff_node *)malloc(sizeof(huff_node));
	retval->left_parent = left;
	retval->right_parent = right;
	retval->count = count;
	retval->value = value;

return retval;
}


huff_dict_entry *new_huff_dict_entry()
{
huff_dict_entry *	retval;

	retval = (huff_dict_entry *)malloc(sizeof(huff_dict_entry));
	retval->bits=0;

return retval;
}



/* Write a 0 or a 1 to a huffman dictionary entry */
void huff_dict_write_bit(huff_dict_entry *to_mod, unsigned char bit_to_add)
{
	/* if it's a 0 we dont need to do anything
	   because the dictionary is already full of 0's. Just increment the 
	   number of bits and exit
	*/

	to_mod->entry[to_mod->bits] = bit_to_add;
	to_mod->bits++;	
}



/* write out dictionary entries to the bit buffer */
void huff_write_out(huff_dict_entry *to_write, char *out_block, unsigned int *g)
{
int	i;

	for( i=to_write->bits -1 ; i>=0; i--) {
		bit_buffer_write(to_write->entry[i],1);

		while (bit_buffer_size()>=8)
			out_block[(*g)++]=((unsigned char) (bit_buffer_read(8)));
	}
}



/*
        mode            - DACT_MODE_COMPR or DACT_MODE_DECMP
                            Determine whether to compress or decompress.
        prev_block      - Previous (uncompressed) block.
        curr_block      - The data to be compressed.
        out_block       - Where to put data after compression.
        blk_size        - Size of prev_block and curr_block.
*/
int comp_huff_algo(int mode, unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size) 
{
	switch(mode) {
		case DACT_MODE_COMPR:
			return(comp_huff_compress(prev_block,curr_block,out_block,blk_size));
			break; /* Heh */
		case DACT_MODE_DECMP:
			return(comp_huff_decompress(prev_block,curr_block,out_block,blk_size));
			break;
		default:
			printf("Unsupported mode: %i\n", mode);
			return(-1);
	}
}



int huff_node_qsort_compare(huff_node **a, huff_node **b)
{
	if( (*a)->count > (*b)->count) return -1;
	if( (*a)->count < (*b)->count) return  1;
  
/* must be equal then */
return 0;
}


int comp_huff_compress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size) 
{
unsigned int	byte_count, i, byte;
unsigned char	*curr_byte;
huff_node	*tree_top[256]; 	/* an array of 256 pointers that always holds top of the tree */
huff_node	*root_node;		/* the root of the tree */
huff_dict_entry	*dictionary[256];	/* the dictionary */
unsigned int	g=1024;			/* index in out_block to start of compressed data */
unsigned int	header[256];		/* the huffman header */
unsigned char	bogus_bits;
	
	/* build the header */
	for( byte=0; byte<=255; byte++) {
		byte_count=0;
		curr_byte=curr_block;
		for(i=0; i<blk_size; i++) { 
			if ( *curr_byte	== byte)
				byte_count++;
			curr_byte++;		
		}
		header[byte]=byte_count;
	}


	root_node = huff_build_tree(header, tree_top, blk_size);
	huff_build_dict(root_node, tree_top, dictionary);

	/* free the tree */
	tree_free(root_node);


	/* save the header here */
	/* urghhh 256 ints - This is REAL stupid as it's not endian safe and 
	   its not processor register size (int) safe. It's only for testing tho :)
	*/
	memcpy(out_block, header, sizeof(header));

	bit_buffer_purge();

	/* Read the buffer and apply the dictionary */
	curr_byte = curr_block;

        for( i=0; i<blk_size; i++) {
		huff_write_out(dictionary[*curr_byte], out_block, &g);
		/* damn I read the example wrong and spent 1/2 an hour 
		   thinking the bit_buffer corresponded to the out block. doh :)
		*/

		while (bit_buffer_size()>=8) 
			out_block[g++]=((unsigned char) (bit_buffer_read(8)));
		curr_byte++;
        }

	/* We may not have the all our compressed data completely aligned on a 
	   char boundary so we'll pad them 
	*/

	bogus_bits = 8 - bit_buffer_size();
	bit_buffer_write(0,bogus_bits);
	out_block[g++]=((unsigned char) (bit_buffer_read(8)));

	/* free the dictionary */
	dict_free(dictionary);
	
return(g);
}



int comp_huff_decompress(unsigned char *prev_block, unsigned char *curr_block, char *out_block, int blk_size) 
{
huff_node       *tree_top[256];         /* an array of 256 pointers that always holds top of the tree */
huff_node       *root_node;             /* the root of the tree */
huff_node       *c_node;                /* current node pointer */
unsigned int    g=1024;                 /* index in curr_block to start of compressed data */
unsigned int	f=0;			/* index in the out_block to the uncompressed data */
unsigned int	header[256];            /* the huffman header */
unsigned char	c_bit;			/* current bit */
 
	/* read in the header */
	memcpy(header, curr_block, sizeof(header));
	
	/* build tree  */
        root_node = huff_build_tree(header, tree_top, DACT_BLK_SIZE);


	/* Start at the root node. If a nodes parent is NULL we have traversed 
	   the tree from bottom to top and decoded the huffman code
	*/

	bit_buffer_purge();

	while(1) {
		c_node = root_node;
		while (c_node->right_parent != NULL) {
			if(bit_buffer_size() == 0)
				bit_buffer_write(curr_block[g++],8);

			c_bit = bit_buffer_read(1);
			if( c_bit == 0) {
				c_node = c_node->right_parent;
			} else { 
				c_node = c_node->left_parent;
			}				
		}
		out_block[f++] = c_node->value;
		if (f == DACT_BLK_SIZE) break;
	}

	/* destroy tree */
	tree_free(root_node);

return f;
}



huff_node *huff_build_tree(int header[256], huff_node *tree_top[256], int blk_size)
{
signed int	i, t;
huff_node       *curr_nodes[256];       /* this one hold pointers to the current level of the tree */
huff_node       *tmp_node;              /* a temporary node pointer ... */
unsigned int	count;
unsigned int	small, small2;

	/*initialise the tree top and the current level on the tree */
	for( i=0; i<256; i++) {
		tree_top[i] = new_huff_node(header[i], NULL, NULL, i);
		curr_nodes[i] = tree_top[i];
	}

        /* go down each level  - 

        */
        for( i=255; i>0; i--) {
		small=0;
		small2=i;

		for( t=0; t<i; t++) {
			if(curr_nodes[t]->count <= curr_nodes[small]->count) {
				small=t;
			}
		}
		
		for( t=0; t<=i; t++) {
			if(curr_nodes[t]->count <= curr_nodes[small2]->count) {
				if (t != small) {
                                	small2=t;
				}
			}
                }

                /* now make that node */
                count = curr_nodes[small]->count + curr_nodes[small2]->count;
                tmp_node = new_huff_node(count, curr_nodes[small], curr_nodes[small2], 0);
                curr_nodes[small]->child = curr_nodes[small2]->child = tmp_node;
                /* curr_nodes holds the current level and we're moving down
                   one so there's one less node - the 2 we added together are no longer in the curr_nodes
                   the new node we created is
                */
		
		if(small2 == i) {
			curr_nodes[small]=tmp_node;
		} else if(small == i) {
			curr_nodes[small2]=tmp_node;
		} else {
			curr_nodes[small]=curr_nodes[i];
			curr_nodes[small2]=tmp_node;
		}
        }

        /* tmp_node is now the root node */

return tmp_node;
}



void huff_build_dict(huff_node *root_node, huff_node *tree_top[256], huff_dict_entry *dictionary[256])
{
int   i;
huff_node       *tmp_node;              /* a temporary node pointer ... */
huff_node       *c_node;                /* current node pointer */

         for( i=0; i<256; i++)
                dictionary[i] = new_huff_dict_entry();

	
        /* Now to build the dictionary */
        for( i=0; i<256; i++) {
                c_node = tree_top[i];
                while (c_node != root_node) {
                        tmp_node = c_node;
                        c_node = c_node->child;
                        /* ok where did we come from? if its left its a 1
                           if it's right a 0
                        */
                        if( c_node->right_parent == tmp_node ) {
                                huff_dict_write_bit(dictionary[i],0);
                        } else {
                                huff_dict_write_bit(dictionary[i],1);
                        }
                }
        }
}



void dict_free(huff_dict_entry *dictionary[256])
{
unsigned int	i;

	for( i=0; i<256; i++)
		free(dictionary[i]);	
}


void tree_free(huff_node *root)
{
	if( root != NULL ) {
		tree_free(root->left_parent);
		tree_free(root->right_parent);
		free(root);
	}
	fflush(stdout);
}
