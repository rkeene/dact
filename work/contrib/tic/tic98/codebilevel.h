/*****************************************************************************
 *
 *   Author:   Stuart Inglis     <singlis@waikato.ac.nz>
 *
 *
 * This file contains the headers relating to bi-level de/compression
 *
 *****************************************************************************/
#ifndef __BILEVEL_H
#define __BILEVEL_H

#include "marklist.h" 

/* must be called before the start of de/compression */
void  bl_clearmodel(void);
void  bl_freemodel(void);

/* writes the template to the stream, and sets up the de/compression params */
void  bl_writetemplate(char inputstr[]);
void  bl_setuptemplate(char inputstr[]);
void  bl_defaulttemplate(void);
void  bl_readtemplate(void);

/* actually performs the de/compression */
void  bl_compress(marktype d,char str[]);
void  bl_progressive_compress(marktype d,char str[]);
void  bl_compress_mark(marktype d);
void  progressive_compress(marktype d);

void  bl_decompress(marktype *d);
void  bl_progressive_decompress(marktype *d);
void  bl_decompress_mark(marktype *d, int w, int h);
void  progressive_decompress(marktype *d);

/* clairvoyantly de/compress d1 with respect to clairvoyantly viewable d2 */
void  bl_clair_compress(marktype d1, marktype d2);
void  bl_clair_compress_2(marktype d1, marktype d2);
void  bl_clair_decompress(marktype d1, marktype d2);

void bl_clearmodel_second();
void  bl_clair_compress_second(marktype d1, marktype d2);

float  bl_compress_mark_return_bits(marktype d);
void  bl_compress_mark_update_only(marktype d);

void  bl_CSPM_init(void);
void  bl_CSPM_free(void);
float  bl_CSPM_return_bits(marktype d, marktype clair);

void  bl_CSPM_update_bits(marktype d, marktype clair);

#endif
