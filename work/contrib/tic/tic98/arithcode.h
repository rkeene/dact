/******************************************************************************
File:		coder.h

Authors: 	John Carpinelli   (johnfc@ecr.mu.oz.au)
	 	Wayne Salamonsen  (wbs@mundil.cs.mu.oz.au)

Purpose:	Data compression using a word-based model and revised 
		arithmetic coding method.

Based on: 	A. Moffat, R. Neal, I.H. Witten, "Arithmetic Coding Revisted",
		Proc. IEEE Data Compression Conference, Snowbird, Utah, 
		March 1995.

Copyright 1995 John Carpinelli and Wayne Salamonsen, All Rights Reserved.

These programs are supplied free of charge for research purposes only,
and may not sold or incorporated into any commercial product.  There is
ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they are
fit for ANY PURPOSE WHATSOEVER.  Use them at your own risk.  If you do
happen to find a bug, or have modifications to suggest, please report
the same to Alistair Moffat, alistair@cs.mu.oz.au.  The copyright
notice above and this statement of conditions must remain an integral
part of each and every copy made of these files.

******************************************************************************/
#ifndef CODER_H
#define CODER_H


#define		CODE_BITS		32
#define		BYTE_SIZE		8
#define 	MAX_BITS_OUTSTANDING	256
#define 	HALF			((unsigned) 1 << (CODE_BITS-1))
#define 	QUARTER			(1 << (CODE_BITS-2))


/* provide external linkage to variables */
extern int f_bits;			/* link to f_bits in stats.c */	
extern unsigned int bytes_input;	/* make available to other modules */
extern unsigned int bytes_output;
extern  long CountOfBitsOut;

/* function prototypes */
void arithmetic_encode(unsigned int l, unsigned int h, unsigned int t);
unsigned int arithmetic_decode_target(unsigned int t);
void arithmetic_decode(unsigned int l, unsigned int h, unsigned int t);
void binary_arithmetic_encode(int c0, int c1, int bit);
int binary_arithmetic_decode(int c0, int c1);
void start_encode(void);
void finish_encode(void);
void start_decode(void);
void finish_decode(void);
void startoutputtingbits(void);
void doneoutputtingbits(void);
void startinputtingbits(void);
void doneinputtingbits(void);

void arith_encode( unsigned int lbnd, unsigned int hbnd, unsigned int totl );
int arith_decode_target( unsigned int totl );
void arith_decode( unsigned int lbnd, unsigned int hbnd, unsigned int totl );

void      EncodeGammaDist(int Off);
int       DecodeGammaDist(void);

void      EncodeGammaSigned(int snum, int *pos, int *neg);
int       DecodeGammaSigned(int *pos, int *neg);

void      InitArithEncoding(void);
void      CloseDownArithEncoding(void);

void      InitArithDecoding(void);
void      CloseDownArithDecoding(void);

void      EncodeChecksum(char str[]);
void      DecodeChecksum(char str[]);

#endif
