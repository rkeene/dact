/*
 *                 (c) Copyright AT&T 1991.
 *
 * License granted for use only in connection with ISO and CCITT evaluations.
 *     AT&T offers no warranties of any kind, expressed or implied.
 *     Contact D. L. Duttweiler (d.l.duttweiler@att.com) for email copies.
 */

/*
 * Version S1R4.1
 */

/*
 * Encoder Usage:
 *	einit()
 *	while("symbols to encode") encoder(symbol,&st[cx],&mps[cx])
 *		where	symbol = 1 or 0
 *			st[NCX] and mps[NCX] are arrays of integers
 *			that are allocated and initialized in
 *			the calling routine.  The dimension NCX must at
 *			least equal the number of contexts.
 *	eflush()
 *
 * Decoder Usage:
 *	dinit()
 *	while(symbols to decode) symbol = decoder(&st[cx],mps[cx])
 *
 * External routines:
 *	ewrite(byte)
 *	byte = dread()
 */



#define NCX		(1<<12)		/* number of contexts */

static int lsz[128] = {
  0x5a1d,0x2586,0x1114,0x080b,0x03d8,0x01da,0x00e5,0x006f,0x0036,
  0x001a,0x000d,0x0006,0x0003,0x0001,0x5a7f,0x3f25,0x2cf2,0x207c,
  0x17b9,0x1182,0x0cef,0x09a1,0x072f,0x055c,0x0406,0x0303,0x0240,
  0x01b1,0x0144,0x00f5,0x00b7,0x008a,0x0068,0x004e,0x003b,0x002c,
  0x5ae1,0x484c,0x3a0d,0x2ef1,0x261f,0x1f33,0x19a8,0x1518,0x1177,
  0x0e74,0x0bfb,0x09f8,0x0861,0x0706,0x05cd,0x04de,0x040f,0x0363,
  0x02d4,0x025c,0x01f8,0x01a4,0x0160,0x0125,0x00f6,0x00cb,0x00ab,
  0x008f,0x5b12,0x4d04,0x412c,0x37d8,0x2fe8,0x293c,0x2379,0x1edf,
  0x1aa9,0x174e,0x1424,0x119c,0x0f6b,0x0d51,0x0bb6,0x0a40,0x5832,
  0x4d1c,0x438e,0x3bdd,0x34ee,0x2eae,0x299a,0x2516,0x5570,0x4ca9,
  0x44d9,0x3e22,0x3824,0x32b4,0x2e17,0x56a8,0x4f46,0x47e5,0x41cf,
  0x3c3d,0x375e,0x5231,0x4c0f,0x4639,0x415e,0x5627,0x50e7,0x4b85,
  0x5597,0x504f,0x5a10,0x5522,0x59eb,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
  0x0000,0x0000};
static int swtch[128] = {
     1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   1,   0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   1,   0,   0,   0,   0,   0,   0,   0,   1,   0,
     0,   0,   0,   0,   0,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
     1,   0,   0,   0,   0,   1,   0,   1,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0};
static int nmps[128] = {
     1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  13,  15,
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,
    31,  32,  33,  34,  35,   9,  37,  38,  39,  40,  41,  42,  43,  44,  45,
    46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,
    61,  62,  63,  32,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,
    76,  77,  78,  79,  48,  81,  82,  83,  84,  85,  86,  87,  71,  89,  90,
    91,  92,  93,  94,  86,  96,  97,  98,  99, 100,  93, 102, 103, 104,  99,
   106, 107, 103, 109, 107, 111, 109, 111,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0};
static int nlps[128] = {
     1,  14,  16,  18,  20,  23,  25,  28,  30,  33,  35,   9,  10,  12,  15,
    36,  38,  39,  40,  42,  43,  45,  46,  48,  49,  51,  52,  54,  56,  57,
    59,  60,  62,  63,  32,  33,  37,  64,  65,  67,  68,  69,  70,  72,  73,
    74,  75,  77,  78,  79,  48,  50,  50,  51,  52,  53,  54,  55,  56,  57,
    58,  59,  61,  61,  65,  80,  81,  82,  83,  84,  86,  87,  87,  72,  72,
    74,  74,  75,  77,  77,  80,  88,  89,  90,  91,  92,  93,  86,  88,  95,
    96,  97,  99,  99,  93,  95, 101, 102, 103, 104,  99, 105, 106, 107, 103,
   105, 108, 109, 110, 111, 110, 112, 112,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0};

long CountOfBitsOut;

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "arithcode.h"
static int byteout();

#define FPC		(void)putc
#define FPF		(void)fprintf
#define E0(A)		{ FPF(stderr,"%s: A\n",prgname); exit(1); }
#define E1(A,B)		{ FPF(stderr,"%s: A\n",prgname,B); exit(1); }
#define E2(A,B,C)	{ FPF(stderr,"%s: A\n",prgname,B,C); exit(1); }
#define E3(A,B,C,D)	{ FPF(stderr,"%s: A\n",prgname,B,C,D);exit(1); }
#define E4(A,B,C,D,E)	{ FPF(stderr,"%s: A\n",prgname,B,C,D,E); exit(1); }


#define EUP		{ *pst = nmps[*pst]; }
#define SUP		{ *pmps = swtch[*pst]? 1-*pmps: *pmps; \
			  *pst = nlps[*pst]; }
#define BYTEIN		{ c += (unsigned)dread()<<8; ct = 8; }
#define RENORME		while(a<0x8000) { \
			  a <<= 1; c <<= 1; ct--; \
			  if(ct==0) (void)byteout(); }
#define RENORMD		do { \
			  if(ct==0) BYTEIN; \
			  a <<= 1; c <<= 1; ct--; } while(a<0x8000);
#define OUT(A)		if(stflag) stflag = 0; else { \
			  if((A)==0) nzero++; \
			  else { \
			    while(nzero>0) {nzero--; (void)ewrite(0x00);} \
			    (void)ewrite(A); } }
#define OUT00		if(stflag) stflag = 0; else nzero++;
#define OUTFF		if(stflag) stflag = 0; else { \
			  while(nzero>0) {nzero--; (void)ewrite(0x00);} \
			  (void)ewrite(0xFF); }

static unsigned c;			/* Coding register */
static int a;				/* Size of coding interval*/
static int stflag;			/* Flag to inhibit first write */
static int nzero;			/* Potential trailing zeros */
static int sc;				/* Number of FF bytes */
static int buffer;			/* Bits buffered for output or input */
static int ct;				/* Shift counter */


#define ATALL		2048		/* minimum AT count */
#define ATMOVE		0x06		/* AT movement */
#define DPMISS		2		/* array value when DP can't predict */
#define ESC		0xff		/* escape */
#define IMMATERIAL	0		/* useful in keeping lint happy */
#define LCLCD		2		/* latency, coloring to coding */

#define ND		6		/* maximum number of delta layers */
#define NM		16		/* maximum AT lag allowed */
#define NP		32		/* maximum number of planes */
#define NPH		4		/* number of phases */
#define NPREFIX		200		/* maximum length of prefix */
#define NS		100		/* maximum number of stripes */
#define NSDE		512		/* SDE transfer size */
#define NWL		4		/* lines in low resolution window */
#define SDNORM		0x02		/* normal stripe data end */
#define SDRST		0x03		/* reset at stripe data end */
#define STUFF		0x00		/* escape escape */
#define TPB2CTX		0x159		/* pseudo context for bottom TP
					   erroneously given as 0x169 in
					   earlier version */
#define TPB3CTX		0x1c9		/* pseudo context for bottom TP */
#define TPDCTX		0xfc3		/* pseudo context for diff TP */
/********************** Derived symbolic constants ****************************/
#define NML		((NM+1)/2)
#define NMH		(2*NML)
#define NWH		(2*NWL)
#define NATC		(NM-1)
/********************** Strings ***********************************************/
#define IRLOOP		for(ir=ndl;ir<=nd;ir++)
#define IPLOOP		for(ip=0;ip<np;ip++)
#define ISLOOP		for(is=0;is<ns;is++)
/********************** Level 0 macros ****************************************/
#define ATCOUNT(A,B)	{ if(atcheck && ixh>=nm && (id!=0 || ixh<nxh-2)) { \
			    if(*php0) { \
			      atc[0] += *(phm1+(A)-2); \
			      for(i=1;i<natc;i++) atc[i] += *(php0-i-B); } \
			    else { \
			      atc[0] += 1-(*(phm1+(A)-2)); \
			      for(i=1;i<natc;i++) atc[i] += 1-(*(php0-i-B)); } \
			    atall++; } }
#define ATSWOK		{ if(atcheck && atall>ATALL) { \
			    atcheck = 0; \
			    cold = atc[iat]; \
			    tmax = 1; lmax = atc[1]; lmin = atc[1]; \
			    for(i=2;i<natc;i++) { \
			      if(atc[i] > lmax) { lmax = atc[i]; tmax = i; } \
			      if(atc[i] < lmin) lmin = atc[i]; } \
			    if(atc[0] >= lmax) { cmax = atc[0]; tmax = 0; } \
			    else cmax = lmax; \
			    cmin = atc[0]<lmin? atc[0]: lmin; \
			    if((8*cmax>7*atall) && \
			       (cmax-cold > atall-cmax) && \
			       (cmax-cold > atall>>4) && \
			       (cmax-(atall-cold) > atall-cmax) && \
			       (cmax-(atall-cold) > atall>>4) && \
			       (cmax-cmin > atall>>2) && \
			       (iat!=0 || (lmax-lmin)>atall>>3)) { \
			      FPF(paf,"id,iyh,new,old = %2d %5d %2d %2d\n", \
				id,iyh,tmax,iat); \
			      FPF(paf,"atall = %4d\natc =",atall); \
			      for(i=0;i<natc;i++) FPF(paf," %4d",atc[i]); \
			      FPF(paf,"\n"); \
			      iatnext = tmax; \
			      yat = 0; } } }
#define FOPEN(A,B,C)	{ STRCPY(file,prefix); STRCAT(file,A); \
			  if((B=fopen(file,C))==NULL) \
			    E1(could not open %s, file); }
#define IOPEN(A,B,C)	{ SPRINTF(file,"%sr%dp%d",prefix,C,ip); \
			  if((A=fopen(file,B))==NULL) \
			    E1(could not open %s,file); }
#define IREAD(A,B)	{ if(rbit==8) { rchar = getc(B); rbit = 0; } \
			  A = (rchar>>(7-rbit++))&1; }
#define IWRITE(A,B)	{ wchar += A<<((7-wbit++)); \
			  if(wbit==8) {FPC(wchar,B); wchar=0; wbit=0;} }
#define MODHGH(A)	imghgh + ((iyh+NWH+(A))%NWH)*nrh + NMH;
#define MODLOW(A)	imglow + ((iyl+NWL+(A))%NWL)*nrl + NML;
#define SDEPARSE	{ id = hitolo? nd+ndl-ir: ir; \
			  sdestrt[ip][id][is] = (int)ftell(pbf); \
			  sdelgth[ip][id][is] = partition(pbf); }
#define SDEWRITE	{ id = hitolo? nd+ndl-ir: ir; \
			  if(fseek(pxf,(long)sdestrt[ip][id][is],0)) \
			    E0(bad seek); \
			  i = sdelgth[ip][id][is]; \
			  while(i>0) { \
			    j = i>NSDE? NSDE: i; \
			    if( fread(sde,1,j,pxf) != j) E0(sde  read error); \
			    if(fwrite(sde,1,j,pbf) != j) E0(sde write error); \
			    i -= j; } }
#define XPUT(A)		{ FPC((int)(A),pxf); }
/********************** Level 1 macros ****************************************/
#define ATMV		{ if(ivh==yat && iat!=iatnext) { \
			    iat = iatnext; nat++; \
			    if(encode) { \
			      XPUT(ESC); XPUT(ATMOVE); \
			      XPUT(0); XPUT(0); \
			      XPUT(ivh>>8); XPUT(ivh&0xff); \
			      XPUT(iatnext>0? iatnext+2: 0); XPUT(0); } } }



FILE *paf,*pbf=stdin,*psf,*pxf=stdout;
int decode,dpon,encode,iatnext,id,ip,lps0,lrltwo,nbyte,nd,nm,nstuff,
  nxtop,nytop,pacfeed,tpbon,tpdon,yat;
char *prefix,*prgname,*strcat(),*strcpy();
void exit();

int ewrite(int c)
{
  nbyte++;
  XPUT(c);
  if(c==ESC) { XPUT(STUFF); nstuff++; }
  return 0;
}

int dread()
{
  int i;
  if(pacfeed) return 0;
  else {
    loop: if((i=getc(pbf))!=ESC) { nbyte++;  return i; }
    else switch(getc(pbf)) {
      case STUFF: nstuff++; nbyte++; return(ESC);
      case ATMOVE: {
        if(getc(pbf)!=0) E0(impossibly large yat);
        if(getc(pbf)!=0) E0(impossibly large yat);
	yat = getc(pbf); yat = (yat<<8) + getc(pbf);
	iatnext = getc(pbf); if(iatnext>0) iatnext -= 2;
        if(getc(pbf)!=0) E0(no support for MY != 0);
	goto loop; }
      case SDNORM:  pacfeed = 1; return 0;
      case SDRST: E0(no support for SDRST);
      case EOF: E0(EOF in dread);
      default: E0(DBID problem in switch); } }
  return 0;				/* keeps lint happy */
}

int decoder(int *pst,int *pmps)
{
  int pix;
  a -= lsz[*pst];
  if((c>>16) < a) {
    if(a<0x8000) { 
      if(a < lsz[*pst]) { pix = 1-*pmps; SUP; } else { pix = *pmps; EUP; }
      RENORMD }
    else pix = *pmps; }
  else {
    if(a<lsz[*pst]) { pix= *pmps;
      c -= (unsigned)a<<16; a=lsz[*pst]; EUP; RENORMD }
    else { pix = 1-*pmps;
      c -= (unsigned)a<<16; a=lsz[*pst]; SUP; RENORMD } }
  return pix;
}

int dinit()
{
  c = 0; BYTEIN; c <<= 8; BYTEIN; c <<= 8; BYTEIN;
  a = 0x10000;
  return 0;
}

int einit()
{
  c = 0; a = 0x10000;
  buffer = 0x00; nzero = 0; stflag = 1; sc = 0; ct = 11;
  return 0;
}

int encoder(int pix,int *pst,int *pmps)
{
  a -= lsz[*pst];
  if(pix==*pmps) { if(a<0x8000) {
    if(a<lsz[*pst]) { c += a; a = lsz[*pst]; }
    EUP; RENORME } }
  else {
    if(a>=lsz[*pst]) { c += a; a = lsz[*pst]; }
    SUP; RENORME }
  return 0;
}

int eflush()
{
  int temp;
  temp = (c+a-1)&0xffff0000;
  if(temp<c) c = temp+0x8000; else c = temp;
  c <<= ct;
  if(c>0x7ffffff) { OUT(buffer+1) while(sc>0) { sc--; OUT00 } }
  else { OUT(buffer) while(sc>0) { sc--; OUTFF } }
  OUT((int)((c>>19)&0xff))
  OUT((int)((c>>11)&0xff))
  return 0;
}

static int byteout()
{
  int temp;
  temp = (c>>19)&0x1ff;
  if(temp>0xff) {
    OUT(buffer+1) while(sc>0) { sc--; OUT00 } buffer = temp&0xff; }
  else {
    if(temp==0xff) sc++;
    else {
      OUT(buffer) while(sc>0) { sc--; OUTFF } buffer = temp; } }
  c &= 0x7ffff; ct = 8;
  return 0;
}


/*

int st[NCX],mps[NCX];

void main(int argc, char *args[])
{
  int i;
  for(i=0;i<NCX;i++)
    st[i]=mps[i]=0;

  if(argc<2){
    fprintf(stderr,"nope, don't know that option, 'e' or 'd' only.\n");
    exit(1);
  }

  if(args[1][0]=='e'){
    putw(100,stdout);
    einit();
    for(i=0;i<100;i++)
      encoder(i%2,&st[0],&mps[0]);
    eflush();
  }
  else  if(args[1][0]=='d'){
    int symbol;
    int len;
    len=getw(stdin);
    dinit();
    fprintf(stderr,"len=%d\n",len);
    for(i=0;i<len;i++){
      symbol = decoder(&st[0],&mps[0]);
      fprintf(stderr,"%d\n",symbol);
    };
  }
  else
    fprintf(stderr,"nope, don't know that option, 'e' or 'd' only.\n");
}


*/
