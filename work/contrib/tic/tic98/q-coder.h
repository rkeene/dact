#ifndef __Q_CODER_H_
#define __Q_CODER_H_

#ifndef Q_CODER
#define Q_CODER
#endif


#define InitArithEncoding() einit()
#define CloseDownArithEncoding() eflush()

#define InitArithDecoding() dinit()
#define CloseDownArithDecoding() {}

#define binary_arithmetic_encode(c0,c1,p) encoder(p,&(c0),&(c1))
#define binary_arithmetic_decode(c0,c1) decoder(&(c0),&(c1))

int decoder(int *st,int *mps);
int dinit(void);

int einit(void);
int encoder(int pix,int *st,int *mps);
int eflush(void);

extern  long CountOfBitsOut;


#endif

