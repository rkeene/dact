#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define R(x) 4*x
#define NORM(x) { x+=0.000001; while(x<1) x*=10; while(x>=1) x/=10; }
#define BEST(x,y,m) { \
 y=((int) (x*8)); \
 y=((((int) (x*pow(10,(y+2))))-(((int) (x*pow(10,(y-1))))*1000))); \
 if (y==999 || y==0) { \
   y=((int) (x*10000))/10; \
 } else { \
   if (y<250 && y>124 && m) { y=((y-125)*8); m=!m; } \
 } \
}
#define CEST(x,y,m) { 				\
y=((int) (x*100000))&0xff;			\
}

char *cipher_chaos_encrypt(const char *src, const char *key, const int srclen) {
	 
}

#ifdef DEBUG_CIPHER_CHAOS
int main(int argc, char **argv) {
	double x;
	int y,m=0,i,imx;
	int hist[1000];

	m=atoi(argv[1]);
	imx=atoi(argv[2]);
	x=m;
	NORM(x);
	printf("Primer=%f (%i iterations)\n",x,imx);
	for (i=0;i<1000;i++) {
		hist[i]=0;
	}
	for (i=0;i<imx;i++) {
		CEST(x,y,m);
		hist[y]++;
		printf("%i %i\n",i,y);
		x=R(x)*(1-x);
		NORM(x);
	}

	for (i=0;i<256;i++) {
		fprintf(stderr, "%i %i\n",i,hist[i]);
	}
}
#endif
