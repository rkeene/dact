/*
 * Module: context
 *
 * Implements the routines for finding image contexts for image
 * compression.
 *
 * This module calculates the normal and clairvoyant JBIG templates. 
 * As well as the ten standard pixels, we introduce a "missing" bit
 * which is set when the template is not entirely within the page.
 * This single bit gives between 1% and 5% compression improvement.
 *
 *
 * Author:
 *   Stuart Inglis (singlis@internz.co.nz)
 *   (c) 1998 
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "globals.h"

#include "arithcode.h"
#include "marklist.h"
#include "pbmtools.h"
#include "template_match.h"

#include "context.h"

coordinate jbig[JBIG_SIZE]={{-1,-2},{0,-2},{1,-2},
			    {-2,-1},{-1,-1},{0,-1},{1,-1},{2,-1},
			    {-2,0},{-1,0}};

coordinate jbig_cl1[MAX_CONTEXT_SIZE],jbig_cl2[MAX_CONTEXT_SIZE];

int JBIG_CLAIR_1,JBIG_CLAIR_2,JBIG_CLAIR_SIZE;


void jbig_setup(int num1, int num2)
{
  JBIG_CLAIR_1=0;
  JBIG_CLAIR_2=0;

  if((num1>MAX_CONTEXT_SIZE)||(num2>MAX_CONTEXT_SIZE)){
    fprintf(stderr,"error: specified context size is larger than the compile time constant.\nThe values '%d' and '%d' should both be <= %d.\n",num1,num2,MAX_CONTEXT_SIZE);
    exit(1);
  }

  if(num1==2){
    JBIG_CLAIR_1=2;
    jbig_cl1[0].x=-1; jbig_cl1[0].y=0;
    jbig_cl1[1].x=0;  jbig_cl1[1].y=-1;
  } else if(num1==4){
    JBIG_CLAIR_1=4;
    jbig_cl1[0].x=-1; jbig_cl1[0].y=0;
    jbig_cl1[1].x=-1; jbig_cl1[1].y=-1;
    jbig_cl1[2].x=0;  jbig_cl1[2].y=-1;
    jbig_cl1[3].x=1;  jbig_cl1[3].y=-1;
  } else if(num1==5){
    JBIG_CLAIR_1=5;
    jbig_cl1[0].x=-2; jbig_cl1[0].y=0;
    jbig_cl1[1].x=-1; jbig_cl1[1].y=0;
    jbig_cl1[2].x=-1; jbig_cl1[2].y=-1;
    jbig_cl1[3].x=0;  jbig_cl1[3].y=-1;
    jbig_cl1[4].x=1;  jbig_cl1[4].y=-1;
  } else if(num1==6){
    JBIG_CLAIR_1=6;
    jbig_cl1[0].x=-2; jbig_cl1[0].y=0;
    jbig_cl1[1].x=-1; jbig_cl1[1].y=0;
    jbig_cl1[2].x=-1; jbig_cl1[2].y=-1;
    jbig_cl1[3].x=0;  jbig_cl1[3].y=-1;
    jbig_cl1[4].x=1;  jbig_cl1[4].y=-1;
    jbig_cl1[5].x=0;  jbig_cl1[5].y=-2;
  } else if(num1==8){
    JBIG_CLAIR_1=8;
    jbig_cl1[0].x=-3; jbig_cl1[0].y=0;
    jbig_cl1[1].x=-2; jbig_cl1[1].y=0;
    jbig_cl1[2].x=-1; jbig_cl1[2].y=0;
    jbig_cl1[3].x=-2; jbig_cl1[3].y=-1;
    jbig_cl1[4].x=-1; jbig_cl1[4].y=-1;
    jbig_cl1[5].x=0;  jbig_cl1[5].y=-1;
    jbig_cl1[6].x=1;  jbig_cl1[6].y=-1;
    jbig_cl1[7].x=0;  jbig_cl1[7].y=-2;
  } else {
    fprintf(stderr,"error: unknown context size '%d'. Should be 2,4,5,6 or 8.\n",num1);
    exit(1);
  }

  if(num2==5){
    JBIG_CLAIR_2=5;
    jbig_cl2[1].x=0;  jbig_cl2[1].y=-1;
    jbig_cl2[0].x=-1; jbig_cl2[0].y=0;
    jbig_cl2[4].x=0;  jbig_cl2[4].y=0;
    jbig_cl2[2].x=1;  jbig_cl2[2].y=0;
    jbig_cl2[3].x=0;  jbig_cl2[3].y=1;
  } else if(num2==7) {
    JBIG_CLAIR_2=7;
    jbig_cl2[0].x=0;  jbig_cl2[0].y=-1;
    jbig_cl2[1].x=-1; jbig_cl2[1].y=0;
    jbig_cl2[2].x=0;  jbig_cl2[2].y=0;
    jbig_cl2[3].x=1;  jbig_cl2[3].y=0;
    jbig_cl2[4].x=-1; jbig_cl2[4].y=1;
    jbig_cl2[5].x=0;  jbig_cl2[5].y=1;
    jbig_cl2[6].x=1;  jbig_cl2[6].y=1;
  } else if(num2==9){
    JBIG_CLAIR_2=9;
    jbig_cl2[0].x=-1; jbig_cl2[0].y=-1;
    jbig_cl2[1].x=0;  jbig_cl2[1].y=-1;
    jbig_cl2[2].x=1;  jbig_cl2[2].y=-1;
    jbig_cl2[3].x=-1; jbig_cl2[3].y=0;
    jbig_cl2[4].x=0;  jbig_cl2[4].y=0;
    jbig_cl2[5].x=1;  jbig_cl2[5].y=0;
    jbig_cl2[6].x=-1; jbig_cl2[6].y=1;
    jbig_cl2[7].x=0;  jbig_cl2[7].y=1;
    jbig_cl2[8].x=1;  jbig_cl2[8].y=1;
  } else if(num2==13){
    JBIG_CLAIR_2=13;
    jbig_cl2[0].x=0;   jbig_cl2[0].y=-2;
    jbig_cl2[1].x=-1;   jbig_cl2[1].y=-1;
    jbig_cl2[2].x=0;   jbig_cl2[2].y=-1;
    jbig_cl2[3].x=1;   jbig_cl2[3].y=-1;
    jbig_cl2[4].x=-2;  jbig_cl2[4].y=0;
    jbig_cl2[5].x=-1;  jbig_cl2[5].y=0;
    jbig_cl2[6].x=0;   jbig_cl2[6].y=0;
    jbig_cl2[7].x=1;   jbig_cl2[7].y=0;
    jbig_cl2[8].x=2;   jbig_cl2[8].y=0;
    jbig_cl2[9].x=-1;  jbig_cl2[9].y=1;
    jbig_cl2[10].x=0;  jbig_cl2[10].y=1;
    jbig_cl2[11].x=1;  jbig_cl2[11].y=1;
    jbig_cl2[12].x=0;  jbig_cl2[12].y=2;
  } else if(num2==15){
    JBIG_CLAIR_2=15;
    jbig_cl2[0].x=0;   jbig_cl2[0].y=-2;
    jbig_cl2[1].x=-1;   jbig_cl2[1].y=-1;
    jbig_cl2[2].x=0;   jbig_cl2[2].y=-1;
    jbig_cl2[3].x=1;   jbig_cl2[3].y=-1;
    jbig_cl2[4].x=-2;  jbig_cl2[4].y=0;
    jbig_cl2[5].x=-1;  jbig_cl2[5].y=0;
    jbig_cl2[6].x=0;   jbig_cl2[6].y=0;
    jbig_cl2[7].x=1;   jbig_cl2[7].y=0;
    jbig_cl2[8].x=2;   jbig_cl2[8].y=0;
    jbig_cl2[9].x=-2;  jbig_cl2[9].y=1;
    jbig_cl2[10].x=-1; jbig_cl2[10].y=1;
    jbig_cl2[11].x=0;  jbig_cl2[11].y=1;
    jbig_cl2[12].x=1;  jbig_cl2[12].y=1;
    jbig_cl2[13].x=2;  jbig_cl2[13].y=1;
    jbig_cl2[14].x=0;  jbig_cl2[14].y=2;
  } else if(num2==25){
    JBIG_CLAIR_2=25;
    jbig_cl2[0].x=0;   jbig_cl2[0].y=-3;
    jbig_cl2[1].x=-1;  jbig_cl2[1].y=-2;
    jbig_cl2[2].x=0;   jbig_cl2[2].y=-2;
    jbig_cl2[3].x=1;   jbig_cl2[3].y=-2;
    jbig_cl2[4].x=-2;  jbig_cl2[4].y=-1;
    jbig_cl2[5].x=-1;  jbig_cl2[5].y=-1;
    jbig_cl2[6].x=0;   jbig_cl2[6].y=-1;
    jbig_cl2[7].x=1;   jbig_cl2[7].y=-1;
    jbig_cl2[8].x=2;   jbig_cl2[8].y=-1;
    jbig_cl2[9].x=-3;  jbig_cl2[9].y=0;
    jbig_cl2[10].x=-2; jbig_cl2[10].y=0;
    jbig_cl2[11].x=-1; jbig_cl2[11].y=0;
    jbig_cl2[12].x=0;  jbig_cl2[12].y=0;
    jbig_cl2[13].x=1;  jbig_cl2[13].y=0;
    jbig_cl2[14].x=2;  jbig_cl2[14].y=0;
    jbig_cl2[15].x=3;  jbig_cl2[15].y=0;
    jbig_cl2[16].x=-2; jbig_cl2[16].y=1;
    jbig_cl2[17].x=-1; jbig_cl2[17].y=1;
    jbig_cl2[18].x=0;  jbig_cl2[18].y=1;
    jbig_cl2[19].x=1;  jbig_cl2[19].y=1;
    jbig_cl2[20].x=2;  jbig_cl2[20].y=1;
    jbig_cl2[21].x=-1; jbig_cl2[21].y=2;
    jbig_cl2[22].x=0;  jbig_cl2[22].y=2;
    jbig_cl2[23].x=1;  jbig_cl2[23].y=2;
    jbig_cl2[24].x=0;  jbig_cl2[24].y=3;
  } else {
    fprintf(stderr,"error: unknown context size '%d'. Should be 5,7,9,13,15 or 25.\n",num2);
    exit(1);
  }

  JBIG_CLAIR_SIZE=JBIG_CLAIR_1+JBIG_CLAIR_2;

  assert(JBIG_CLAIR_1>0);
  assert(JBIG_CLAIR_2>0);
  assert(JBIG_CLAIR_SIZE>2);
}


#define DO_HASH

#ifdef DO_HASH
#define HASH(x) (((x^(((x+(x&7))*713))))%1047551)
#define MAXVALUE (1047551)
#else
#define HASH(x) (x)
#endif



void
context_init(contexttype *c, int size)
{
  assert(c);
  assert(size>=1);
  
  jbig_setup(globals.g_num1, globals.g_num2);

  c->size=size;
#ifdef DO_HASH
  c->max_value=MAXVALUE;
#else
  c->max_value=ROUND(pow((double)CONTEXT_BITS,(double)size));
#endif

  c->p=(probtype*)calloc(c->max_value,sizeof(probtype));
  assert(c->p);
  context_clear(c);
}


void 
context_free(contexttype *c)
{
  assert(c);

  if(c->p)
    free(c->p);
  c->p=NULL;
}

void
context_clear(contexttype *c)
{
  int i;

  assert(c);

  for(i=0;i<c->max_value;i++)
    c->p[i].w = c->p[i].b=globals.g_context_init;
}

void
context_update(contexttype *c, int mask, int pixel)
{
  assert(c);
  assert(mask>=0);
  assert(mask<c->max_value);
  assert((pixel==0)||(pixel==1));

  if(pixel){
    c->p[mask].b+=globals.g_context_increment;
  } else {
    c->p[mask].w+=globals.g_context_increment;
  }
  
  if((c->p[mask].b + c->p[mask].w) > globals.g_context_norm){
    c->p[mask].b=(c->p[mask].b+1)/2;
    c->p[mask].w=(c->p[mask].w+1)/2;
  }
}



void 
context_encode(contexttype *c, int mask, int p)
{
  assert(c);
  assert((mask>=0) && (mask<c->max_value));
  assert((p==0)||(p==1));

  binary_arithmetic_encode(c->p[mask].w, c->p[mask].b, p);
}

float 
context_prob(contexttype *c, int mask, int p)
{
  float f;

  assert(c);
  assert((mask>=0) && (mask<c->max_value));
  assert((p==0) || (p==1));

  if(p){
    f=(c->p[mask].b)/(1.0*(c->p[mask].b+c->p[mask].w));
  } else {
    f=(c->p[mask].w)/(1.0*(c->p[mask].b+c->p[mask].w));
  }
  return f;
}

int
context_decode(contexttype *c, int mask)
{
  int p;
  assert(c);
  assert((mask>=0) && (mask<c->max_value));

  p=binary_arithmetic_decode(c->p[mask].w, c->p[mask].b);
  return p;
}





int 
jbig_mask(marktype *m,int x, int y)
{
  static int i,p, ti,tj;
  static long long mask;

  assert(m);

  mask=0;

  for(i=0;i<JBIG_SIZE;i++){
    ti=x+jbig[i].x;
    tj=y+jbig[i].y;
    if((tj<0)||(ti<0)||(ti>=m->w)||(tj>=m->h))
#ifdef BINARY
      p=0;
#else
      p=CONTEXT_BITS-1;
#endif
    else
      p=pbm_getpixel((m->bitmap),ti,tj);
    mask=CONTEXT_BITS*mask + p;
  }
  return HASH(mask);
}

int 
jbig_mask_clair(marktype *m,marktype *clair,
		    int xc_off,int yc_off,
		    int x, int y)
{
  static int i,ti,tj, p;
  static long long mask;

  assert(m);
  assert(clair);

  mask=0;

  for(i=0;i<JBIG_CLAIR_1;i++){
    ti=x+jbig_cl1[i].x;
    tj=y+jbig_cl1[i].y;
    if((tj<0)||(ti<0)||(ti>=m->w)||(tj>=m->h))
#ifdef BINARY
      p=0;
#else
      p=CONTEXT_BITS-1;
#endif
    else
      p=pbm_getpixel((m->bitmap),ti,tj);
    mask=CONTEXT_BITS*mask + p;
  }
  for(i=0;i<JBIG_CLAIR_2;i++){
    ti=x+jbig_cl2[i].x-xc_off;
    tj=y+jbig_cl2[i].y-yc_off;
    if((tj<0)||(ti<0)||(ti>=clair->w)||(tj>=clair->h))
#ifdef BINARY
      p=0;
#else
      p=CONTEXT_BITS-1;
#endif
    else
      p=pbm_getpixel((clair->bitmap),ti,tj);
    mask=CONTEXT_BITS*mask + p;
  }
  return HASH(mask);
}



void 
compress_mark(contexttype *con, marktype *m, int st) 
{
  int i,j;
  int mask,p;

  assert(con);
  assert(m);

  for(j=0;j<m->h;j++){
    for(i=0;i<m->w;i++){
      mask=jbig_mask(m,i,j);
      p=pbm_getpixel((m->bitmap),i,j);
      if(st==COMPRESS)
	context_encode(con,mask,p);
#ifndef Q_CODER
      context_update(con,mask,p);
#endif
    }
  }
}


void 
compress_mark_clair(contexttype *con, marktype *m, marktype *cl,
			 int xco, int yco, 
			 int st) 
{
  int i,j;
  int mask,p;
  
  assert(con);
  assert(m);
  assert(cl);

  for(j=0;j<m->h;j++){
    for(i=0;i<m->w;i++){
      mask=jbig_mask_clair(m,cl,xco,yco,i,j);
      p=pbm_getpixel((m->bitmap),i,j);
      if(st==COMPRESS)
	context_encode(con,mask,p);
#ifndef Q_CODER
      context_update(con,mask,p);
#endif
    }
  }
}


void 
decompress_mark(contexttype *con, marktype *m, int w,int h)
{
  int i,j;
  int mask,p;

  assert(con);
  assert(m);

  marktype_alloc(m,w,h);

  for(j=0;j<m->h;j++){
    for(i=0;i<m->w;i++){
      mask=jbig_mask(m,i,j);
      p=context_decode(con,mask);
      pbm_putpixel((m->bitmap),i,j,p);
#ifndef Q_CODER
      context_update(con,mask,p);
#endif
    }
  }
}



void 
decompress_mark_clair(contexttype *con, marktype *m, marktype *cl,
			   int w,int h,
			   int xco, int yco)
{
  int i,j;
  int mask,p;

  assert(con);
  assert(m);
  assert(cl);

  marktype_alloc(m,w,h);

  for(j=0;j<m->h;j++){
    for(i=0;i<m->w;i++){
      mask=jbig_mask_clair(m,cl,xco,yco,i,j);
      p=context_decode(con,mask);
      pbm_putpixel((m->bitmap),i,j,p);
#ifndef Q_CODER
      context_update(con,mask,p);
#endif
    }
  }
}

