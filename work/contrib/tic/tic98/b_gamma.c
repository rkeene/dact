/*
 * Module: b_gamma
 * 
 * This module implements a gamma-type encoding using a
 * binary arithmetic coder. The encoder() and decoder()
 * modules correspond to the standard qm-coder routines,
 * but here they are stubs to a normal arithmetic coder.
 *
 * The counts are updated using the Dirichlet estimator
 * and are normalised into 8 bits.
 *
 * Author:
 *   Stuart Inglis (singlis@internz.co.nz)
 *   (c) 1998
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "arithcode.h"
#include "globals.h"

#include "b_gamma.h"

#ifndef Q_CODER

static void 
encoder(int p, int *c0, int *c1)
{
  binary_arithmetic_encode(*c0,*c1,p);
  if(p){
    (*c1)+=globals.g_gamma_increment;
  } else {
    (*c0)+=globals.g_gamma_increment;
  }
  if((*c0)+(*c1)>=globals.g_gamma_norm){
    (*c0)=((*c0)+1)/2;
    (*c1)=((*c1)+1)/2;
  }
}

static int 
decoder(int *c0, int *c1)
{
  int p;
  p=binary_arithmetic_decode(*c0,*c1);
  if(p){
    (*c1)+=globals.g_gamma_increment;
  } else {
    (*c0)+=globals.g_gamma_increment;
  }
  if((*c0)+(*c1)>=globals.g_gamma_norm){
    (*c0)=((*c0)+1)/2;
    (*c1)=((*c1)+1)/2;
  }
  return p;
}
#endif



b_gammaptr 
b_gamma_init(void )
{
    b_gammaptr q;
    q=(b_gammaptr)malloc(sizeof(b_gammatype));
    assert(q);

#ifndef Q_CODER
    q->st=q->mps=globals.g_gamma_init;    
#else
    q->st=q->mps=0;
#endif

    q->left=q->right=NULL;
    return q;
}

void 
b_gamma_free(b_gammaptr q)
{
  if(q->left){
    b_gamma_free(q->left);
  } 
  if(q->right){
    b_gamma_free(q->right);
  }
  free(q);
}

#define Q_GOLEFT(q) {if(q->left==NULL) q->left=b_gamma_init();q=q->left;}
#define Q_GORIGHT(q) {if(q->right==NULL) q->right=b_gamma_init(); q=q->right;}



void 
b_gamma_encode(b_gammaptr q, int num)
{
    int t,len,start,enc;

    /* first encode the sign */

    if(num>=0){
      encoder(0,&(q->st),&(q->mps)); Q_GORIGHT(q);
    } else {
      num++;
      num=-num;
      encoder(1,&(q->st),&(q->mps)); Q_GOLEFT(q);
    }

    /* now encode the length */

    t=num+1;
    len=-1;
    while(t>0){
      t/=2;
      len++;
    }
    
    for(t=0;t<len;t++){
      encoder(0,&(q->st),&(q->mps)); Q_GORIGHT(q);
    }
    encoder(1,&(q->st),&(q->mps));   Q_GOLEFT(q);

    start=(1<<len)-1;
    enc=num-start;
     
    for(t=0;t<len;t++){
      if(enc & (1<<(len-t-1))){
	encoder(1,&(q->st),&(q->mps)); Q_GOLEFT(q);
      } else {
	encoder(0,&(q->st),&(q->mps)); Q_GORIGHT(q);
      }
    }
}






int 
b_gamma_decode(b_gammaptr q)
{
    int t,len;
    int neg,d,dec,start;

    /* first dec the sign */

    neg=decoder(&(q->st),&(q->mps));

    if(neg==0){
      Q_GORIGHT(q);
    } else {
      Q_GOLEFT(q);
    }

    /* now encode the length */

    len=0;
    while((d=decoder(&(q->st),&(q->mps)))==0){
      Q_GORIGHT(q);
      len++;
    }
    Q_GOLEFT(q);
    
    start=(1<<len)-1;
     
    dec=0;
    for(t=0;t<len;t++){
      d=decoder(&(q->st),&(q->mps));
      if(d==0){
	Q_GORIGHT(q);
      } else {
	Q_GOLEFT(q);
      }
      dec=dec*2 + d;
    }

    if(neg==0){
      return start + dec;
    } else {
      return -(start+dec+1);
    }
}

#ifdef 0
void 
b_gamma_encode_64(unsigned long num)
{
  int i;

  for(i=63;i>=0;i--){
    binary_arithmetic_encode(1,1,num&((unsigned long)1<<i));
  }
}

unsigned long
b_gamma_decode_64()
{
  unsigned long num=0;
  int i;

  for(i=0;i<64;i++){
    num=num*2 + binary_arithmetic_decode(1,1);
  }
  return num;
}

void 
b_gamma_encode_32(unsigned long num)
{
  int i;

  for(i=31;i>=0;i--){
    binary_arithmetic_encode(1,1,num&((unsigned int)1<<i));
  }
}

unsigned long
b_gamma_decode_32()
{
  unsigned long num=0;
  int i;

  for(i=0;i<32;i++){
    num=num*2 + binary_arithmetic_decode(1,1);
  }
  return num;
}
#endif


void 
b_gamma_encode_ll(b_gammaptr q, long long num)
{
    long long t,len,start,enc;

    /* first encode the sign */

    if(num>=0){
      encoder(0,&(q->st),&(q->mps)); Q_GORIGHT(q);
    } else {
      num++;
      num=-num;
      encoder(1,&(q->st),&(q->mps)); Q_GOLEFT(q);
    }

    /* now encode the length */

    t=num+1;
    len=-1;
    while(t>0){
      t/=2;
      len++;
    }
    
    for(t=0;t<len;t++){
      encoder(0,&(q->st),&(q->mps)); Q_GORIGHT(q);
    }
    encoder(1,&(q->st),&(q->mps));   Q_GOLEFT(q);

    start=((long long)1<<len)-1;
    enc=num-start;
     
    for(t=0;t<len;t++){
      if(enc & ((long long)1<<(len-t-1))){
	encoder(1,&(q->st),&(q->mps)); Q_GOLEFT(q);
      } else {
	encoder(0,&(q->st),&(q->mps)); Q_GORIGHT(q);
      }
    }
}






long long
b_gamma_decode_ll(b_gammaptr q)
{
    long long t,len;
    long long neg,d,dec,start;

    /* first dec the sign */

    neg=decoder(&(q->st),&(q->mps));

    if(neg==0){
      Q_GORIGHT(q);
    } else {
      Q_GOLEFT(q);
    }

    /* now encode the length */

    len=0;
    while((d=decoder(&(q->st),&(q->mps)))==0){
      Q_GORIGHT(q);
      len++;
    }
    Q_GOLEFT(q);
    
    start=((long long)1<<len)-1;
     
    dec=0;
    for(t=0;t<len;t++){
      d=decoder(&(q->st),&(q->mps));
      if(d==0){
	Q_GORIGHT(q);
      } else {
	Q_GOLEFT(q);
      }
      dec=dec*2 + d;
    }

    if(neg==0){
      return start + dec;
    } else {
      return -(start+dec+1);
    }
}



#undef Q_GOLEFT
#undef Q_GORIGHT



