#ifndef _arithcode_h
#define _arithcode_h
/*
	Declarations for arithmetic coding.
	Alistair Moffat, alistair@cs.mu.oz.au, July 1987.

	Added opening/closing and binary coding prototypes
	Stuart Inglis, singlis@internz.co.nz, January 1998.
*/

#include <stdlib.h>

#define codevaluebits 16
#if (codevaluebits<16)
typedef unsigned short codevalue;
#else
typedef unsigned long  codevalue;
#endif




extern  FILE *arith_in,*arith_out;
extern  codevalue	S_low, S_high, S_value;
extern  long		S_bitstofollow;
extern  int		S_buffer, S_bitstogo;
extern  long		cmpbytes, rawbytes;
extern  long CountOfBitsOut;




#define topvalue ((codevalue)((1<<codevaluebits)-1))
#define maxfrequency ((topvalue+1)/4 + 1)
#define firstqtr (topvalue/4+1)
#define half     (2*firstqtr)
#define thirdqtr (3*firstqtr)
#define escape_event	U->totalcnt-U->notfound, U->totalcnt, U->totalcnt
#define	arithmetic_decode_target(totl)	\
	(  ((S_value-S_low+1)*(totl)-1) / (S_high-S_low+1)  )


void      arithmetic_encode(int,int,int);
/*int arithmetic_decode_target(void); */
void      arithmetic_decode(int,int,int);


void      InitArithEncoding(void);
void      CloseDownArithEncoding(void);

void      InitArithDecoding(void);
void      CloseDownArithDecoding(void);

void binary_arithmetic_encode(int c0, int c1, int p);
int binary_arithmetic_decode(int c0, int c1);

#endif

