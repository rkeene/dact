#ifndef _TIC98_H
#define _TIC98_H

#include "marklist.h"
#include "codebook.h"
#include "context.h"
#include "b_gamma.h"



#define NEW_PAGE         0 
#define END_OF_DOCUMENT  1


#define INTCODETYPE         b_gammaptr
#define INTCODE_INIT()      b_gamma_init()
#define INTCODE_FREE(p)     b_gamma_free(p)
#define INTCODE_ENCODE(p,q) b_gamma_encode(p,q)
#define INTCODE_DECODE(p)   b_gamma_decode(p)

#define INTCODE_ENCODE_LL(p,q) b_gamma_encode_ll(p,q)
#define INTCODE_DECODE_LL(p)   b_gamma_decode_ll(p)

#define START_ENC()         InitArithEncoding()
#define FINISH_ENC()        CloseDownArithEncoding()
#define START_DEC()         InitArithDecoding()
#define FINISH_DEC()        {}


typedef struct {
  INTCODETYPE  misc,z_w,z_h,z_rw,z_rh,p_cl,xpos,ypos,cx,cy;
  codebook c;
  contexttype global,clairall;
} tic98type;


void        tic98_compress_marks(tic98type *t, marklistptr list, int w, int h);
marklistptr tic98_decompress_marks(tic98type *t, int *w, int *h);


void   tic98_init(tic98type *t);
void   tic98_free(tic98type *t);

void   tic98_start_encoding(void);
void   tic98_finish_encoding(void);

void   tic98_start_decoding(void);
void   tic98_finish_decoding(void);

void   tic98_status(tic98type *t, FILE *fp);

void   tic98_num_encode(tic98type *t, int num);
int    tic98_num_decode(tic98type *t);

#endif



