/******************************************************************************
File:		coder.c

Authors: 	John Carpinelli   (johnfc@ecr.mu.oz.au)
	 	Wayne Salamonsen  (wbs@mundil.cs.mu.oz.au)

Purpose:	Data compression using a word-based model and revised 
		arithmetic coding method.

Based on: 	A. Moffat, R. Neal, I.H. Witten, "Arithmetic Coding Revisted",
		Proc. IEEE Data Compression Conference, Snowbird, Utah, 
		March 1995.

		Low-Precision Arithmetic Coding Implementation by 
		Radford M. Neal



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
#include <stdio.h>
#include <assert.h>
#include "arithcode.h"



long CountOfBitsOut;


unsigned long	L;				/* lower bound */
unsigned long	R;				/* code range */
unsigned long	V;				/* current code value */
unsigned long 	r;				/* normalized range */

int 		bits_outstanding;		/* follow bit count */
int	 	buffer;				/* I/O buffer */
int		bits_to_go;			/* bits left in buffer */
unsigned int	bytes_input, bytes_output;	/* I/O counters */


/*
 *
 * responsible for outputing the bit passed to it and an opposite number of
 * bit equal to the value stored in bits_outstanding
 *
 */
#define BIT_PLUS_FOLLOW(b)		\
do                                      \
{ 	  			        \
    OUTPUT_BIT((b));           		\
    while (bits_outstanding > 0)	\
    { 					\
	OUTPUT_BIT(!(b));      		\
	bits_outstanding -= 1;    	\
    } 	                		\
} while (0)


/*
 *
 * responsible for outputting one bit. adds the bit to a buffer 
 * and once the buffer has 8 bits, it outputs a character
 *
 */
#define OUTPUT_BIT(b)            	\
do { 					\
    buffer >>= 1;             		\
    if (b) 				\
	buffer |= 1 << (BYTE_SIZE-1);	\
    bits_to_go -= 1;            	\
    if (bits_to_go == 0)        	\
    { 					\
	putc(buffer, stdout); 		\
	bytes_output += 1;		\
        bits_to_go = BYTE_SIZE;      	\
    }	                       		\
    CountOfBitsOut++;		\
} while (0)


/*
 *
 * reads in bits from encoded file 
 * reads in a char at a time into a buffer i.e 8 bits
 *
 */
#define ADD_NEXT_INPUT_BIT(v) 		\
do { 					\
    bits_to_go -= 1;			\
    if (bits_to_go < 0) 		\
    { 					\
	buffer = getc(stdin);		\
	bits_to_go = BYTE_SIZE - 1;	\
    } 					\
    v += v + (buffer & 1); 		\
    buffer >>= 1; 			\
} while (0) 

/*
 * output code bits until the range as been expanded
 * to above QUARTER
 */
#define ENCODE_RENORMALISE		\
do {					\
    while (R < QUARTER)			\
    {					\
        if (L >= HALF)			\
    	{				\
    	    BIT_PLUS_FOLLOW(1);		\
    	    L -= HALF;			\
    	}				\
    	else if (L+R <= HALF)		\
    	{				\
    	    BIT_PLUS_FOLLOW(0);		\
    	}				\
    	else 				\
    	{				\
    	    bits_outstanding++;		\
    	    L -= QUARTER;		\
    	}				\
    	L += L;				\
    	R += R;				\
    }					\
} while (0)


/*
 * input code bits until range has been expanded to
 * more than QUARTER. Mimics encoder.
 */
#define DECODE_RENORMALISE		\
do {					\
    while (R < QUARTER)			\
    {					\
    	if (L >= HALF)			\
    	{				\
    	    V -= HALF;			\
    	    L -= HALF;			\
    	    bits_outstanding = 0;	\
    	}				\
    	else if (L+R <= HALF)		\
    	{				\
    	    bits_outstanding = 0;	\
    	}				\
    	else				\
    	{				\
    	    V -= QUARTER;		\
    	    L -= QUARTER;		\
    	    bits_outstanding++;		\
    	}				\
    	L += L;				\
    	R += R;				\
    	ADD_NEXT_INPUT_BIT(V);		\
    }					\
} while (0)



/*
 *
 * encode a symbol given its low, high and total frequencies
 *
 */
void 
arithmetic_encode(unsigned int low, unsigned int high, unsigned int total)
{
    unsigned long temp; 

#ifndef SHIFT_ADD
    r = R/total;
    temp = r*low;
    L += temp;
    if (high < total)
	R = r*(high-low);
    else
	R -= temp;
#else
{
    int i, nShifts;
    unsigned long numerator, denominator;
    unsigned long temp2;

    /*
     * calculate r = R/total, temp = r*low and temp2 = r*high
     * using shifts and adds 
     */
    numerator = R;
    nShifts = CODE_BITS - f_bits - 1;
    denominator = total << nShifts;
    r = 0;
    temp = 0;
    temp2 = 0;
    for (i = nShifts;; i--) 
    {
        if (numerator >= denominator) 
	{ 
	    numerator -= denominator; 
	    r++; 
	    temp += low;
	    temp2 += high;
	}
	if (i == 0) break;
        numerator <<= 1; r <<= 1; temp <<= 1; temp2 <<= 1;
    }
    L += temp;
    if (high < total)
	R = temp2 - temp;
    else
	R -= temp;
}
#endif

    ENCODE_RENORMALISE;

    if (bits_outstanding >= MAX_BITS_OUTSTANDING)
    {
	finish_encode();
	start_encode();
    }
}



/*
 *
 * decode the target value using the current total frequency
 * and the coder's state variables
 *
 */
unsigned 
int arithmetic_decode_target(unsigned int total)
{
    unsigned long target;
    
#ifndef SHIFT_ADD
    r = R/total;
    target = (V-L)/r;
#else 
{	
    int i, nShifts;
    unsigned long numerator, denominator;

    /* divide r = R/total using shifts and adds */
    numerator = R;
    nShifts = CODE_BITS - f_bits - 1;
    denominator = total << nShifts;
    r = 0;
    for (i = nShifts;; i--) 
    {
        if (numerator >= denominator) 
	{ 
	    numerator -= denominator; 
	    r++; 
	}
	if (i == 0) break;
        numerator <<= 1; r <<= 1;
    }

    /* divide V-L by r using shifts and adds */
    if (r < (1 << (CODE_BITS - f_bits - 1)))
	nShifts = f_bits;
    else
	nShifts = f_bits - 1;
    numerator = V - L;
    denominator = r << nShifts;
    target = 0;
    for (i = nShifts;; i--) 
    {
        if (numerator >= denominator) 
	{ 
	    numerator -= denominator; 
	    target++; 
	}
	if (i == 0) break;
        numerator <<= 1; target <<= 1;
    }
}
#endif
    return (target >= total? total-1 : target);
}



/*
 *
 * decode the next input symbol
 *
 */
void 
arithmetic_decode(unsigned int low, unsigned int high, unsigned int total)
{     
    unsigned int temp;

#ifndef SHIFT_ADD
    /* assume r has been set by decode_target */
    temp = r*low;
    L += temp;
    if (high < total)
	R = r*(high-low);
    else
	R -= temp;
#else
{
    int i, nShifts;
    unsigned long temp2;
    
    /* calculate r*low and r*high using shifts and adds */
    r <<= f_bits;
    temp = 0;
    nShifts = CODE_BITS - f_bits - 1;
    temp2 = 0;
    for (i = nShifts;; i--) 
    {
	if (r >= HALF)
	{ 
	    temp += low;
	    temp2 += high;
	}
	if (i == 0) break;
        r <<= 1; temp <<= 1; temp2 <<= 1;
    }
    L += temp;
    if (high < total)
	R = temp2 - temp;
    else
	R -= temp;
 }
#endif

    DECODE_RENORMALISE;

    if (bits_outstanding >= MAX_BITS_OUTSTANDING)
    {
	finish_decode();	
	start_decode();
    }
}



/*
 * 
 * encode a binary symbol using specialised binary encoding
 * algorithm
 *
 */
void
binary_arithmetic_encode(int c0, int c1, int bit)
{
    int LPS, cLPS, rLPS;

    if (c0 < c1) 
    {
	LPS = 0;
	cLPS = c0;
    } else {
	LPS = 1;
	cLPS = c1;
    }
#ifndef SHIFT_ADD
    r = R / (c0+c1);
    rLPS = r * cLPS;
#else
{	
    int i, nShifts;
    unsigned long int numerator, denominator;

    numerator = R;
    nShifts = CODE_BITS - f_bits - 1;
    denominator = (c0 + c1) << nShifts;
    r = 0;
    rLPS = 0;
    for (i = nShifts;; i--) 
    {
	if (numerator >= denominator) 
	{ 
	    numerator -= denominator; 
	    r++;
	    rLPS += cLPS;
	}
	if (i == 0) break;
	numerator <<= 1; r <<= 1; rLPS <<= 1;
    }
}
#endif
    if (bit == LPS) 
    {
	L += R - rLPS;
	R = rLPS;
    } else {
	R -= rLPS;
    }

    /* renormalise, as for arith_encode */
    ENCODE_RENORMALISE;

    if (bits_outstanding > MAX_BITS_OUTSTANDING)
    {
	finish_encode();
	start_encode();
    }
}



/*
 *
 * decode a binary symbol given the frequencies of 1 and 0 for
 * the context
 *
 */
int
binary_arithmetic_decode(int c0, int c1)
{
    int LPS, cLPS, rLPS, bit;

    if (c0 < c1) 
    {
	LPS = 0;
	cLPS = c0;
    } else {
	LPS = 1;
	cLPS = c1;
    }
#ifndef SHIFT_ADD
    r = R / (c0+c1);
    rLPS = r * cLPS;
#else 
{
    int i, nShifts;
    unsigned long int numerator, denominator;

    numerator = R;
    nShifts = CODE_BITS - f_bits - 1;
    denominator = (c0 + c1) << nShifts;
    r = 0;
    rLPS = 0;
    for (i = nShifts;; i--) 
    {
	if (numerator >= denominator) 
	{ 
	    numerator -= denominator; 
	    r++;
	    rLPS += cLPS;
	}
	if (i == 0) break;
	numerator <<= 1; r <<= 1; rLPS <<= 1;
    }
}
#endif
    if ((V-L) >= (R-rLPS)) 
    {
	bit = LPS;
	L += R - rLPS;
	R = rLPS;
    } else {
	bit = (1-LPS);
	R -= rLPS;
    }

    /* renormalise, as for arith_decode */
    DECODE_RENORMALISE;

    if (bits_outstanding > MAX_BITS_OUTSTANDING)
    {
	finish_decode();	
	start_decode();
    }
    return(bit);
}




/*
 *
 * start the encoder
 *
 */
void 
start_encode(void)
{
    L = 0;
    R = HALF-1;
    bits_outstanding = 0;
}



/*
 *
 * finish encoding by outputting follow bits and three further
 * bits to make the last symbol unambiguous
 * could tighten this to two extra bits in some cases,
 * but does anybody care?
 *
 */
void 
finish_encode(void)
{
    int bits, i;
    const int nbits = 3;

    bits = (L+(R>>1)) >> (CODE_BITS-nbits);
    for (i = 1; i <= nbits; i++)     	/* output the nbits integer bits */
        BIT_PLUS_FOLLOW(((bits >> (nbits-i)) & 1));
}



/*
 *
 * start the decoder
 *
 */
void 
start_decode(void)
{
    int 	i;
    static	fill_V = 1;
    
    if (fill_V)
    {
	V = 0;
	for (i = 0; i<CODE_BITS; i++)
	    ADD_NEXT_INPUT_BIT(V);
	fill_V = 0;
    }
    L = 0;
    R = HALF - 1;
    bits_outstanding = 0;
}


/*
 *
 * finish decoding by consuming the disambiguating bits generated
 * by finish_encode
 *
 */
void 
finish_decode(void)
{
    int i;
    const int nbits = 3;

    for (i = 1; i <= nbits; i++)
	ADD_NEXT_INPUT_BIT(V);	
    bits_outstanding = 0;
}


/*
 *
 * initialize the bit output function
 *
 */
void 
startoutputtingbits(void)
{
    buffer = 0;
    bits_to_go = BYTE_SIZE;
}


/*
 *
 * start the bit input function
 *
 */
void 
startinputtingbits(void)
{
    bits_to_go = 0;
}



/*
 *
 * complete outputting bits
 *
 */
void 
doneoutputtingbits(void)
{
    putc(buffer >> bits_to_go, stdout);
    bytes_output += 1;
}


/*
 *
 * complete inputting bits
 *
 */
void 
doneinputtingbits(void)
{
    bits_to_go = 0;
}

#ifdef 0
void
arith_encode( unsigned int lbnd, unsigned int hbnd, unsigned int totl )
/* Arithmetically encode the range. */
{
    if ((lbnd < 0) || (hbnd < 0) || (totl < 0))
        fprintf( stderr, "Fatal error - invalid range : lbnd %d hbnd %d totl %d\n", lbnd, hbnd, totl );
    assert( (lbnd >= 0) && (hbnd >= 0) && (totl >= 0) );

    if ((totl < hbnd) || (hbnd < lbnd) || ((hbnd == lbnd) && (hbnd != 0)))
        fprintf( stderr, "Fatal error - invalid range : lbnd %d hbnd %d totl %d\n", lbnd, hbnd, totl );
    assert ((totl >= hbnd) && (hbnd >= lbnd) && ((hbnd != lbnd) || (hbnd == 0)));

    if ((lbnd == 0) && (hbnd == totl)) {
	/* probability = 1 - no need to encode it */
    }
    else
	arithmetic_encode( lbnd, hbnd, totl );
}
#endif

int
arith_decode_target( unsigned int totl )
/* Arithmetically decodes the target. */
{
    int target;

    if (totl == 0) /* only occurs at start of decoding */
	return (0);

    target = arithmetic_decode_target( totl );
    return( arithmetic_decode_target( totl ));
}


#ifdef 0
void arith_decode( unsigned int lbnd, unsigned int hbnd, unsigned int totl )
/* Arithmetically decode the range. */
{
    if ((lbnd < 0) || (hbnd < 0) || (totl < 0))
        fprintf( stderr, "Fatal error - invalid range : lbnd %d hbnd %d totl %d\n", lbnd, hbnd, totl );
    assert( (lbnd >= 0) && (hbnd >= 0) && (totl >= 0) );
    if ((lbnd == 0) && (hbnd == totl)) {
        /* probility = 1 - no need to decode it */
    }
    else {
        arithmetic_decode( lbnd, hbnd, totl );
    }
}

#endif











void 
EncodeGammaDist (int Off)
/* Encode Off with probability distribution derived from the Elias gamma distribution. */
{
  int Len, i;

  Off++;
  for (Len = 31; Len >= 0 && !(Off & (1 << Len)); Len--);

  for (i = Len; i >= 0; i--)
    arithmetic_encode (0, 1, 2);
  arithmetic_encode (1, 2, 2);

  for (i = Len - 1; i >= 0; i--)
    if (Off & (1 << i))
      arithmetic_encode (1, 2, 2);
    else
      arithmetic_encode (0, 1, 2);
}



int 
DecodeGammaDist (void)
/* Decode next Off using probability distribution derived from the Elias gamma distribution. Inverse of EncodeGammaDist. */
{
  int i = 1, Len = 0, bit;

  do {
    bit = arithmetic_decode_target (2) < 1 ? 0 : 1;
    if (bit)
      arithmetic_decode (1, 2, 2);
    else {
      arithmetic_decode (0, 1, 2);
      Len++;
    }
  }
  while (!bit);

  Len--;
  while (Len-- > 0) {
    bit = arithmetic_decode_target (2) < 1 ? 0 : 1;
    if (bit)
      arithmetic_decode (1, 2, 2);
    else
      arithmetic_decode (0, 1, 2);
    i = (i << 1) | bit;
  }
  return (i - 1);
}




#ifdef 0

void 
EncodeGammaSigned (int snum, int *pos, int *neg)
{
  if (snum >= 0)
    arithmetic_encode (0, *pos, *pos + (*neg)), (*pos)++;
  else
    arithmetic_encode (*pos, *pos + (*neg), *pos + (*neg)), (*neg)++;
  if (*pos + (*neg) >= HALF) {
    *pos = (*pos + 1) / 2;
    *neg = (*neg + 1) / 2;
  }
  EncodeGammaDist (abs (snum));
}


int 
DecodeGammaSigned (int *pos, int *neg)
{
  int signedpos = 1;
  if (arithmetic_decode_target (*pos + (*neg)) < *pos)
    arithmetic_decode (0, *pos, *pos + (*neg)), (*pos)++;
  else
    arithmetic_decode (*pos, *pos + (*neg), *pos + (*neg)), (*neg)++, signedpos = 0;

  if (*pos + (*neg) >= HALF) {
    *pos = (*pos + 1) / 2;
    *neg = (*neg + 1) / 2;
  }
  if (signedpos)
    return DecodeGammaDist ();
  else
    return (-DecodeGammaDist ());
}

#endif

void 
InitArithEncoding (void)
{
  start_encode();
  startoutputtingbits();
}

void 
InitArithDecoding (void)
{
  start_decode();
  startinputtingbits();
}

void 
CloseDownArithEncoding (void)
{
  finish_encode();
  doneoutputtingbits();
}

void 
CloseDownArithDecoding (void)
{
  finish_decode();
  doneinputtingbits();
}





/* encode that famous number, 42 uses 12 bits */
void 
EncodeChecksum (char str[])
{
/*    if(V) fprintf(stderr,"encoding '%s' checksum...\n",str); */
  EncodeGammaDist (42);
}

void 
DecodeChecksum (char str[])
{
  if (DecodeGammaDist () != 42) {
    fprintf (stderr, "'%s' checksum error, file corrupt.\n", str);
/*      exit(1); */
  } else {
/*      if(V) fprintf(stderr,"'%s' checksum correct...\n",str); */
  }
}

