#ifndef _B_GAMMA_H
#define _B_GAMMA_H

#define BUFFER_SIZE 128

typedef struct b_gammatype *b_gammaptr;
typedef struct b_gammatype {
  int st, mps;
  b_gammaptr left,right;
}b_gammatype;

b_gammaptr  b_gamma_init(void);
void        b_gamma_free(b_gammaptr q);
void        b_gamma_encode(b_gammaptr q, int num);
int         b_gamma_decode(b_gammaptr q);

void        b_gamma_encode_ll(b_gammaptr q, long long num);
long long   b_gamma_decode_ll(b_gammaptr q);

void b_gamma_encode_64(unsigned long num);
unsigned long b_gamma_decode_64();

void b_gamma_encode_32(unsigned long num);
unsigned long b_gamma_decode_32();


#endif



