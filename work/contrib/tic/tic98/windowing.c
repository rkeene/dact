/*
 * Module: windowing
 *
 * Routines for reading/writing and convolving.
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

#include "windowing.h"
#include "utils.h"
#include <assert.h>



double window_rectangular_1D(int i, int N)
{
  i=i; /* to quieten compilers */
  N=N;
  return 1.0;
}

double window_circular_1D(int i, int N)
{
    if((i>=0) && (i<=N))
	return 1.0;
    else
	return 0.0;
}

double window_bartlett_1D(int i, int N)
{
  if((i>=0) && (i<=N)){
    if(N==0)
      return 1.0;
    else
      return 1.0-i/(double)N;
  }
  else 
    return 0.0;
}

double window_hanning_1D(int i, int N)
{
  if((i>=0) && (i<=N)){
    if(N==0)
      return 1.0;
    else
      return (1.0-cos(PI*(1.0+i/(double)(N))))/2.0;
  }
  else
    return 0.0;
}


double window_hamming_1D(int i, int N)
{
  if((i>=0) && (i<=N)){
    if(N==0)
      return 1.0;
    else 
      return (0.54-0.46*cos(PI*(1.0+i/(double)(N))));
  }
  else
    return 0.08;
}


double window_blackman_1D(int i, int N)
{
  if((i>=0) && (i<N)){
    if(N==0)
      return 1.0;
    else 
      return (0.42-0.5*cos(PI*(1.0+i/(double)(N)))+0.08*
	    cos(2.0*PI*(1.0+i/(double)(N))));
  }
  else
    return 0.0;
}


/* sd is a percentage of N/2, ie. 0.5 = 50% of N/2 */
double window_gaussian_1D(int i, int N, double sd)
{
    sd=N*sd;
    
    if(N==0)
      return 1.0;
    else 
      return exp(-( (double)i*i)/(2.0*sd*sd));
}







void create_window(
		   double *window,
		   int width,
		   int TYPE
		   )

{
    int r,i;

    assert(window);
    assert(width>0);

    if((width%2)==0)
	fprintf(stderr,"comment: usually recommended to have an 'odd' size window width\n");

    for(r=0;r<width;r++){
	i=abs(r-width/2);
	switch (TYPE) {
	  case RECTANGULAR: window[r]=window_rectangular_1D(i,width/2);
	    break;
	  case BARTLETT:    window[r]=window_bartlett_1D(i,width/2);
	    break;
	  case HANNING:     window[r]=window_hanning_1D(i,width/2);
	    break;
	  case HAMMING:     window[r]=window_hamming_1D(i,width/2);
	    break;
	  case BLACKMAN:    window[r]=window_blackman_1D(i,width/2);
	    break;
	  case GAUSSIAN:    window[r]=window_gaussian_1D(i,width/2,0.4);
	    break;
	default:{
	      fprintf(stderr,"create_window: don't know this window type!\n");
	      exit(1);
	    }
	}
    }

}

void create_window_2D(
		   double **window,
		   int width,
		   int TYPE
		   )

{
    int i,j;
    int r;
    int width_r;

    if((width%2)==0)
	fprintf(stderr,"comment: usually recommended to have an "
		"odd size window width\n");

    /* circumscribe */
    width_r=ROUND(sqrt((width/2*width/2)));

    /* at corners */
    /*    width_r=ROUND(sqrt((width/2*width/2) * 2));*/

    for(j=0;j<width;j++)
	for(i=0;i<width;i++){
	    r=ROUND(sqrt((double)(j-width/2)*(j-width/2)+
			 (double)(i-width/2)*(i-width/2)));
	    switch (TYPE) {
	      case RECTANGULAR: window[j][i]=window_rectangular_1D(r,width_r);
		break;
	      case BARTLETT:    window[j][i]=window_bartlett_1D(r,width_r);
		break;
	      case HANNING:     window[j][i]=window_hanning_1D(r,width_r);
		break;
	      case HAMMING:     window[j][i]=window_hamming_1D(r,width_r);
		break;
	      case BLACKMAN:    window[j][i]=window_blackman_1D(r,width_r);
		break;
	      case GAUSSIAN:    window[j][i]=window_gaussian_1D(r,width_r,0.4);
		break;
	    default:{
		  fprintf(stderr,"create_window: don't know this window type!\n");
		  exit(1);
		}
	    }
	}
}


void write_window(
		  char *filename,
		  double *window,
		  int L,
		  int H
		  )

{
    int i;
    FILE *fp;
    int open=0;

    assert(window);
    assert(H>=L);

    if((!filename) || (strcmp(filename,"-")==0)){
	fp=(stdout);
    } else{
	fp=fopen(filename,"wb");
	if(!fp) {
	    fprintf(stderr,"write_window: can't create file '%s'\n",filename);
	    return ;
	}
	open=1;
    }

    for(i=L;i<=H;i++)
	fprintf(fp,"%d %g\n",i,window[i]);
    
    fflush(fp);
    if(open)
	fclose(fp);
}


void write_window_labelled(
		  char *filename,
		  double *window,
		  int L,
		  int H,
		  double lowrange,
		  double highrange,
                  double bin
		  )

{
    int i;
    double r;
    FILE *fp;
    int open=0;

    assert(window);
    assert(H>=L);

    highrange=highrange; /* to quieten compilers */

    if(bin<=0){
	fprintf(stderr,"write_window_labelled: warning, binsize<=0\n");
    }

    if((!filename) || (strcmp(filename,"-")==0))
	fp=(stdout);
    else{
	fp=fopen(filename,"wb");
	if(!fp) {
	    fprintf(stderr,"write_window: can't create file '%s'\n",filename);
	    return;
	}
	open=1;
    }

    r=lowrange;
    for(i=L ;i<=H; i++,	r+=bin){
/*	
	r=lowrange+(i-L)*(highrange-lowrange)/(H-L);
*/
	if(bin<0.01)
	    fprintf(fp,"%.3f %g\n",r,window[i]);
	else if(bin<0.1)
	    fprintf(fp,"%.2f %g\n",r,window[i]);
	else if(bin<1)
	    fprintf(fp,"%.1f %g\n",r,window[i]);
	else 
	    fprintf(fp,"%g %g\n",r,window[i]);
    }
    
    fflush(fp);
    if(open)
	fclose(fp);
}


void write_window_2D(
		  char *filename,
		  double **window,
		  int Ly,
		  int Hy,
		  int Lx,
		  int Hx
		  )

{
    int i,j;
    FILE *fp;
    int open=0;
    
    if((!filename) || (strcmp(filename,"-")==0))
	fp=(stdout);
    else{
	fp=fopen(filename,"wb");
	if(!fp) {
	    fprintf(stderr,"write_window: can't create file '%s'\n",filename);
	    return;
	}
	open=1;
    }
    
    for(j=Ly;j<=Hy;j++)
	for(i=Lx;i<=Hx;i++)
	    fprintf(fp,"%d\t%d\t%f\n",i,j,window[j][i]);
    
    fflush(fp);
    if(open)
	fclose(fp);
}


void write_window_2D_pgm_P2(
		  char *filename,
		  double **window,
		  int Ly,
		  int Hy,
		  int Lx,
		  int Hx,
		  int maxval,
		  char *commentstring
		  )

{
    double **copy;
    int j,i;
    int open=0;
    FILE *fp;

    CALLOC_2D(copy,Hy-Ly+1,Hx-Lx+1,double);

    for(j=Ly;j<=Hy;j++)
	for(i=Lx;i<=Hx;i++)
	    copy[j-Ly][i-Lx]=window[j][i];

    normalise_window_2D(copy,Ly,Hy,Lx,Hx);


    if((!filename) || (strcmp(filename,"-")==0)){
	fp=(stdout);
    } else{
	fp=fopen(filename,"wb");
	if(!fp) {
	    fprintf(stderr,"write_window_2D_pgm: can't create file '%s'\n",filename);
	    return;
	}
	open=1;
    }

    if(commentstring){
      if(commentstring[strlen(commentstring)-1]=='\n')
	commentstring[strlen(commentstring)-1]=0;
    }
    if(maxval<=255){
      fprintf(fp,"P5\n");
      if(commentstring){
	fprintf(fp,"# %s\n",commentstring);
      }
      fprintf(fp,"%d %d\n",Hx-Lx+1,Hy-Ly+1);
      fprintf(fp,"%d\n",maxval);
    } else {
      fprintf(fp,"P2\n");
      if(commentstring){
	fprintf(fp,"# %s\n",commentstring);
      }
      fprintf(fp,"%d %d\n",Hx-Lx+1,Hy-Ly+1);
      fprintf(fp,"%d\n",maxval);
    }
    
    for(j=Ly;j<=Hy;j++){
	for(i=Lx;i<=Hx;i++){
	    if((int)(copy[j-Ly][i-Lx]*maxval)<0)
		fprintf(stderr,"warning - file '%s' has negative values!!!\n",filename);
	    if(maxval<=255){
	      fputc((int)(copy[j-Ly][i-Lx]*maxval),fp);
	    } else {
	      fprintf(fp,"%d ",(int)(copy[j-Ly][i-Lx]*maxval));
	    }
	}
	if(maxval>255){
	  fprintf(fp,"\n");
	}
    }
    
    fflush(fp);
    if(open)
	fclose(fp);

    FREE_2D(copy,Hy-Ly+1);
    
}




void write_window_2D_pgm_P2_float(
		  char *filename,
		  float **window,
		  int Ly,
		  int Hy,
		  int Lx,
		  int Hx,
		  int maxval
		  )

{
    double **copy;
    int j,i;
    int open=0;
    FILE *fp;

    CALLOC_2D(copy,Hy-Ly+1,Hx-Lx+1,double);

    for(j=Ly;j<=Hy;j++)
	for(i=Lx;i<=Hx;i++)
	    copy[j-Ly][i-Lx]=window[j][i];

    normalise_window_2D(copy,Ly,Hy,Lx,Hx);


    if((!filename) || (strcmp(filename,"-")==0))
	fp=(stdout);
    else{
	fp=fopen(filename,"wb");
	if(!fp) {
	    fprintf(stderr,"write_window_2D_pgm: can't create file '%s'\n",filename);
	    return;
	}
	open=1;
    }

    fprintf(fp,"P2\n");
    fprintf(fp,"%d %d\n",Hx-Lx+1,Hy-Ly+1);
    fprintf(fp,"%d\n",maxval);
    
    for(j=Ly;j<=Hy;j++){
	for(i=Lx;i<=Hx;i++){
	    if((int)(copy[j-Ly][i-Lx]*maxval)<0)
		fprintf(stderr,"warning - file '%s' has negative values!!!\n",filename);
	    fprintf(fp,"%d ",(int)(copy[j-Ly][i-Lx]*maxval));
	}
	fprintf(fp,"\n");
    }
    
    fflush(fp);
    if(open)
	fclose(fp);

    FREE_2D(copy,Hy-Ly+1);
}


void write_window_2D_pgm_P5_float(
		  char *filename,
		  float **window,
		  int Ly,
		  int Hy,
		  int Lx,
		  int Hx,
		  int maxval
		  )

{
    double **copy;
    int j,i;
    int open=0;
    FILE *fp;

    CALLOC_2D(copy,Hy-Ly+1,Hx-Lx+1,double);

    for(j=Ly;j<=Hy;j++)
	for(i=Lx;i<=Hx;i++)
	    copy[j-Ly][i-Lx]=window[j][i];

    normalise_window_2D(copy,Ly,Hy,Lx,Hx);


    if((!filename) || (strcmp(filename,"-")==0))
	fp=(stdout);
    else{
	fp=fopen(filename,"wb");
	if(!fp) {
	    fprintf(stderr,"write_window_2D_pgm: can't create file '%s'\n",filename);
	    return;
	}
	open=1;
    }

    fprintf(fp,"P5\n");
    fprintf(fp,"%d %d\n",Hx-Lx+1,Hy-Ly+1);
    fprintf(fp,"%d\n",maxval);
    
    for(j=Ly;j<=Hy;j++){
	for(i=Lx;i<=Hx;i++){
	  if((int)(copy[j-Ly][i-Lx]*maxval)<0)
	    fprintf(stderr,"warning - file '%s' has negative values!!!\n",filename);
	  fputc((char)(copy[j-Ly][i-Lx]*maxval),fp);
	}
    }
    
    fflush(fp);
    if(open)
      fclose(fp);
    
    FREE_2D(copy,Hy-Ly+1);
}



	
void normalise_window(
		      double *window,
		      int L,
		      int H
		      )

{
    int i;
    double maxval,minval;
    double gap;


    minval=maxval=window[L];

    for(i=L; i<=H; i++){
	if(window[i]>maxval) 
	    maxval=window[i];
	if(window[i]<minval) 
	    minval=window[i];
    }

    gap=maxval-minval;
    if(gap!=0){
	for(i=L; i<=H; i++)
	    window[i]=(window[i]-minval)/gap;
    }
}

void normalise_window_2D(
		      double **window,
		      int Ly,
		      int Hy,
		      int Lx,
		      int Hx
		      )

{
    int i,j;
    double maxval,minval;
    double gap;
    
    minval=maxval=window[Ly][Lx];

    for(j=Ly; j<=Hy;j++)
	for(i=Lx; i<=Hx; i++){
	    if(window[j][i]>maxval) 
		maxval=window[j][i];
	    if(window[j][i]<minval) 
		minval=window[j][i];
	}
    
    gap=maxval-minval;
    
    if(gap!=0){
	for(j=Ly; j<=Hy;j++)
	    for(i=Lx; i<=Hx; i++){
		window[j][i]=(window[j][i]-minval)/gap;
	    }
    }

    for(j=Ly; j<=Hy;j++)
	for(i=Lx; i<=Hx; i++)
	    if(window[j][i]<0){
		fprintf(stderr,"warning - there are negative values in this window\n");return;}

}

void convolve(
	      double *data,
	      int L,
	      int H,
	      double *window,
	      int width,
	      int origin,
	      double *result,
	      int OPTIONS
	      )

{
    int n,k;
    double x,h;
    double sum;
    int b;
    
    for(n=L; n<=H ;n++){
	sum=0;
	for(k=-origin; k<width-origin; k++){
	    if(OPTIONS&CYCLIC){
		b=n+k;
		while(b<L) b+=(H-L+1);
		while(b>H) b-=(H-L+1);
		x=data[b];
	    }
	    else {
		if((n+k<L) || (n+k>H))
		    x=0.0;
		else
		    x=data[n+k];
	    }
	    
	    h=window[k+origin];
	    
	    sum+=x*h;
	}
	result[n]=sum;
    }
}


void convolve_2D(
	      double **data,
	      int Ly,
	      int Hy,
	      int Lx,
	      int Hx,
	      double **window,
	      int width,
	      int originy,
	      int originx,
	      double **result,
	      int OPTIONS
	      )

{
    int i,j;
    int lx,ly;
    double x;
    double sum;
    int bx,by;


    for(j=Ly;j<=Hy;j++)
	for(i=Lx; i<=Hx ;i++){
	    sum=0;

	    for(ly=-originy;ly<width-originy;ly++){
		for(lx=-originx;lx<width-originx;lx++){
		    if(OPTIONS&CYCLIC){
			bx=i+lx;
			while(bx<Lx) bx+=(Hx-Lx+1);
			while(bx>Hx) bx-=(Hx-Lx+1);
			by=j+ly;
			while(by<Ly) by+=(Hy-Ly+1);
			while(by>Hy) by-=(Hy-Ly+1);
			x=data[by][bx]*
			    window[ly+originy][lx+originx];
		    }
		    else {
			if((j+ly<Ly) || (j+ly>Hy) || (i+lx<Lx) || (i+lx>Hx))
			    x=0;
			else
			    x=data[j+ly][i+lx] *
				window[ly+originy][lx+originx];
		    }
		sum+=x;
		}
	    }
	    result[j][i]=sum;
	}
}

    
