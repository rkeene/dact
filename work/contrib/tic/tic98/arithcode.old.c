/*
 * Arithmetic encoding. Taken largely from Witten, Neal, Cleary,
 * CACM, 1987, p520.
 * Includes bitfiddling routines.
 * Alistair Moffat, alistair@cs.mu.oz.au, January 1987.
 *
 * Adding opening/closing routines to setup global variables,
 * and binary_coding functions. Update to ANSI C++
 * Stuart Inglis, singlis@internz.co.nz, January 1998.
 * 
 */
#include <stdio.h>
#include <assert.h>
#include "arithcode.h"
#include "utils.h"


FILE *arith_in = stdin, *arith_out = stdout;

#undef BECAREFUL
#define INTEGRATED
/*#undef INTEGRATED*/


codevalue S_low = 0, S_high = 0, S_value = 0;
long S_bitstofollow = 0;
int S_buffer = 0, S_bitstogo = 0;

long cmpbytes = 0, rawbytes = 0;

#ifdef INTEGRATED
long CountOfBitsOut;
#endif


/*==================================*/

#define BITPLUSFOLLOW(b)		\
{	OUTPUTBIT((b));			\
	while (bitstofollow)		\
	{	OUTPUTBIT(!(b));	\
		bitstofollow -= 1;	\
	}				\
}

/*
 * {    if(putc(buffer, arith_out)==EOF) /
 * error("OUTPUTBIT","file write error",""); \
 */

#ifdef INTEGRATED
#define OUTPUTBIT(b)			\
{	buffer >>= 1;			\
	if (b) buffer |= 0x80;	 	\
	bitstogo -= 1;			\
	if (bitstogo==0)		\
	{	putc(buffer, arith_out); \
		bitstogo = 8;		\
		cmpbytes += 1;		\
	}				\
	CountOfBitsOut++;		\
}
#else
#define OUTPUTBIT(b)			\
{	buffer >>= 1;			\
	if (b) buffer |= 0x80;	 	\
	bitstogo -= 1;			\
	if (bitstogo==0)		\
	{	putc(buffer, arith_out);\
	        bitstogo = 8;  \
                cmpbytes += 1; \
        } \
}

#endif

#define ADDNEXTINPUTBIT(v)		\
{	if (bitstogo==0)		\
	{	buffer = getc(arith_in);	\
		bitstogo = 8;		\
	}				\
	v = (v<<1)+(buffer&1);		\
	buffer >>= 1;			\
	bitstogo -=1;			\
}

/*==================================*/

void
arithmetic_encode (int lbnd, int hbnd, int totl)
{
  register codevalue low, high;

#ifdef BECAREFUL
  if (!(0 <= lbnd && lbnd < hbnd && hbnd <= totl && totl < maxfrequency)) {
    fprintf (stderr, "(corrupt file) coding error: %d %d %d\n", lbnd, hbnd, totl);
    abort ();
  }
#endif

  {
    register codevalue range;
    range = S_high - S_low + 1;
    high = S_low + range * hbnd / totl - 1;
    low = S_low + range * lbnd / totl;
  }

  {
    register codevalue H;
    register long bitstofollow;
    register int buffer, bitstogo;

    bitstofollow = S_bitstofollow;
    buffer = S_buffer;
    bitstogo = S_bitstogo;
    H = half;

    for (;;) {
      if (high < H) {
	BITPLUSFOLLOW (0);
      } else if (low >= H) {
	BITPLUSFOLLOW (1);
	low -= H;
	high -= H;
      } else if (low >= firstqtr && high < thirdqtr) {
	bitstofollow++;
	low -= firstqtr;
	high -= firstqtr;
      } else
	break;
      low += low;
      high += high;
      high++;
    }
    S_bitstofollow = bitstofollow;
    S_buffer = buffer;
    S_bitstogo = bitstogo;
    S_low = low;
    S_high = high;
  }
}

/*==================================*/

/* Made into a macro 
 * 
 * codevalue
 * arithmetic_decode_target(totl)
 * register int totl;
 * {    return (((S_value-S_low+1)*totl-1) / (S_high -S_low+1));
 * } 
 */

/*==================================*/

void
arithmetic_decode (int lbnd, int hbnd, int totl)
{
  register codevalue low, high;

#ifdef BECAREFUL
  if (!(0 <= lbnd && lbnd < hbnd && hbnd <= totl && totl < maxfrequency)) {
    fprintf (stderr, "(corrupt file) coding error: %d %d %d\n", lbnd, hbnd, totl);
    abort ();
  }
#endif

  {
    register codevalue range;
    range = S_high - S_low + 1;
    high = S_low + range * hbnd / totl - 1;
    low = S_low + range * lbnd / totl;
  }

  {
    register codevalue value, H;
    register int buffer, bitstogo;

    buffer = S_buffer;
    bitstogo = S_bitstogo;
    H = half;
    value = S_value;

    for (;;) {
      if (high < H) {		/* nothing */
      } else if (low >= H) {
	value -= H;
	low -= H;
	high -= H;
      } else if (low >= firstqtr && high < thirdqtr) {
	value -= firstqtr;
	low -= firstqtr;
	high -= firstqtr;
      } else
	break;
      low += low;
      high += high;
      high += 1;
      ADDNEXTINPUTBIT (value);
    }
    S_buffer = buffer;
    S_bitstogo = bitstogo;
    S_low = low;
    S_high = high;
    S_value = value;
  }
}

/*==================================*/


static void
startoutputingbits (void)
{
  S_buffer = 0;
  S_bitstogo = 8;
}

static void
startinputingbits (void)
{
  S_buffer = 0;
  S_bitstogo = 0;
}

static void
doneoutputingbits (void)
{				/* write another byte if necessary */
  if (S_bitstogo < 8) {
    putc (S_buffer >> S_bitstogo, arith_out);
    cmpbytes += 1;
  }
}

/*==================================*/

static void
startencoding (void)
{
  S_low = 0;
  S_high = topvalue;
  S_bitstofollow = 0;
}

static void
startdecoding (void)
{
  register int i;
  register int buffer, bitstogo;
  S_low = 0;
  S_high = topvalue;
  S_value = 0;
  bitstogo = S_bitstogo;
  buffer = S_buffer;
  for (i = 0; i < codevaluebits; i++)
    ADDNEXTINPUTBIT (S_value);
  S_bitstogo = bitstogo;
  S_buffer = buffer;
}

static void
doneencoding (void)
{
  register long bitstofollow;
  register int buffer, bitstogo;
  bitstofollow = S_bitstofollow;
  buffer = S_buffer;
  bitstogo = S_bitstogo;
  bitstofollow += 1;
  if (S_low < firstqtr) {
    BITPLUSFOLLOW (0);
  } else {
    BITPLUSFOLLOW (1);
  }
  S_bitstofollow = bitstofollow;
  S_buffer = buffer;
  S_bitstogo = bitstogo;
}




void 
InitArithEncoding (void)
{
  S_low = 0, S_high = 0, S_value = 0;
  S_bitstofollow = 0;
  S_buffer = 0, S_bitstogo = 0;
  cmpbytes = 0, rawbytes = 0;

  startoutputingbits ();
  startencoding ();
}

void 
InitArithDecoding (void)
{
  S_low = 0, S_high = 0, S_value = 0;
  S_bitstofollow = 0;
  S_buffer = 0, S_bitstogo = 0;
  cmpbytes = 0, rawbytes = 0;

  startinputingbits ();
  startdecoding ();
}

void 
CloseDownArithEncoding (void)
{
  doneencoding ();
  doneoutputingbits ();
}

void 
CloseDownArithDecoding (void)
{
}



void 
binary_arithmetic_encode(int c0, int c1, int p)
{
  assert(c0>0);
  assert(c1>0);
  if(p==0){
    arithmetic_encode(0, c0, c0+c1);
  } else {
    arithmetic_encode(c0, c0+c1, c0+c1);
  }
}

int 
binary_arithmetic_decode(int c0, int c1)
{
  int p,t;
  assert(c0>0);
  assert(c1>0);
  if((t=arithmetic_decode_target(c0+c1))<c0){
    p=0;
    arithmetic_decode(0, c0, c0+c1);
  } else {
    p=1;
    arithmetic_decode(c0, c0+c1, c0+c1);
  }
  return p;
}
  
