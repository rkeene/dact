/*****************************************************************************
 *
 *   Author:   Stuart Inglis     <singlis@waikato.ac.nz>
 *
 *****************************************************************************/
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "marklist.h"
#include "math-utils.h"
#include "windowing.h"
#include "hough.h"

#include "double_array.h"


#define BOTTOM_CENTRE

#if defined(CENTROID)
char method[]="centroid position";
#elif defined(BOTTOM_CENTRE)
char method[]="bottom centre";
#else 
#error
#endif


#define TYPE float
#define MAX_DIVS (18003)

double sin_values[MAX_DIVS],cos_values[MAX_DIVS];


/* Calculate the Hough transform in the range HS..HF in NUM_BINS steps */
void
hough_calc_range(
		 marklistptr list, 
		 int image_w, 
		 int image_h, 
		 double HS,
		 double HF, 
		 double NUM_BINS,
		 int div_amount,
		 char raw_filename[],
		 int uselog)
{
  int omega,r, rmax;
  double rr;
  TYPE **accum=NULL;
  TYPE maxaccum=0;
  double **save_pgm=NULL;
  marklistptr step;
  int hs,hf;
  double xd,yd,centre_x,centre_y;
  doublearraytype ddd;

  HS=HS+90;
  HF=HF+90;

  if(HF-HS==0.0) {
    fprintf(stderr,"skew_hough: gap too small.\n");
    exit(1);
  }
  da_init(&ddd,HS,HF,(HF-HS)/NUM_BINS);

  hs=0;
  hf=da_idx(&ddd,HF);

  if((hf-hs)>=(MAX_DIVS-1)) {
    fprintf(stderr,"wanting %d, max you can have is %d!\n",hf-hs,MAX_DIVS);
    exit(1);
  }

  if(((8*sizeof(TYPE)))>=(marklist_length(list))){
    fprintf(stderr,"WARNING -- possible accumulator array overflow -- %d points!!!\n",marklist_length(list));
  }

  centre_x = image_w/2.0;
  centre_y = image_h/2.0;
  rmax = (int)(sqrt((double)(image_w * image_w + image_h * image_h))/2.0  /div_amount);

  /* allocate the Hough accumulator */
  CALLOC_2D(accum,2*rmax+1, hf-hs+1,TYPE); 
  
  /* setup the sine/cosine table */
  for (omega = hs; omega<hf; omega++){
    rr=da_avg(&ddd,omega);
    sin_values[omega-hs] = sin((double)DEG2RAD(rr));
    cos_values[omega-hs] = cos((double)DEG2RAD(rr));
  }
  

  /* map each component into the Hough accumulator space */
  for(step=list; step; step=step->next){
    float increment=1;
    TYPE *pos;

#ifdef CENTROID
    xd=ROUND(step->data.xpos+step->data.d_xcen);
    yd=ROUND(step->data.ypos+step->data.d_ycen);
#elif defined(BOTTOM_CENTRE)
    xd=step->data.xpos+step->data.d_xcen;
    yd=step->data.ypos+step->data.h-1;
#endif
      
    /* for each component, draw the sinusoid in Hough space */
    for (omega = hs; omega < hf; omega++){

      /* centre_y-yd because +ve skew is anticlockwise from east */
      rr = (centre_y-yd)*sin_values[omega-hs] + (xd-centre_x)*cos_values[omega-hs];

      /* div_amount is the scale of r */
      rr=rr/div_amount;

      /* clamp into the Hough space */
      r=rmax+ROUND(rr);
      if(r<0) {
	r=0;
      } else {
	if(r>=2*rmax+1) {
	  r=2*rmax+1-1;
	}
      }

      pos=&accum[r][omega-hs];

      /* update the accumulator with log(number of pixels) */
      if(uselog){
	increment=log(step->data.set);
      } 
	

      (*pos)+=increment;

      if((*pos)>maxaccum){
	maxaccum=(*pos);
      }
    }
  }

  /* Write out original accumulator array */
  CALLOC_2D(save_pgm,2*rmax+1, hf-hs+1,double); 
  
  for(r=0;r<2*rmax+1;r++)
    for(omega=hs; omega<hf;omega++)
      save_pgm[r][omega-hs]=accum[r][omega-hs];

  /* store as a P2 with proper maximum so we know the original range */
  {
    char commentstring[1000];
    sprintf(commentstring,"Components: HS %g, HF %g, NUM_BINS %g, r_div %d, uselog %d\n",HS,HF,NUM_BINS,div_amount,uselog);
    write_window_2D_pgm_P2(raw_filename,save_pgm,0,2*rmax,0,hf-hs-1,ROUND(maxaccum),commentstring);
  }
  FREE_2D(save_pgm,2*rmax+1);
  
  da_free(&ddd);

  FREE_2D(accum,2*rmax+1);
}





/* Calculate the Hough transform in the range HS..HF in NUM_BINS steps */
void
hough_image_calc_range(
		       marktype *image, 
		       double HS,
		       double HF, 
		       double NUM_BINS,
		       int div_amount,
		       char raw_filename[])
{
  int omega,r, rmax;
  double rr;
  TYPE **accum=NULL;
  TYPE maxaccum=0;
  double **save_pgm=NULL;
  int hs,hf;
  int x,y;
  double centre_x,centre_y;
  doublearraytype ddd;
  float increment=1;
  TYPE *pos;


  HS=HS+90;
  HF=HF+90;

  if(HF-HS==0.0) {
    fprintf(stderr,"skew_hough: gap too small.\n");
    exit(1);
  }
  da_init(&ddd,HS,HF,(HF-HS)/NUM_BINS);

  hs=0;
  hf=da_idx(&ddd,HF);

  if((hf-hs)>=(MAX_DIVS-1)) {
    fprintf(stderr,"wanting %d, max you can have is %d!\n",hf-hs,MAX_DIVS);
    exit(1);
  }

  centre_x = image->w/2.0;
  centre_y = image->h/2.0;
  rmax = (int)(sqrt((double)(image->w * image->w + image->h * image->h))/2.0  /div_amount);

  /* allocate the Hough accumulator */
  CALLOC_2D(accum,2*rmax+1, hf-hs+1,TYPE); 
  
  /* setup the sine/cosine table */
  for (omega = hs; omega<hf; omega++){
    rr=da_avg(&ddd,omega);
    sin_values[omega-hs] = sin((double)DEG2RAD(rr));
    cos_values[omega-hs] = cos((double)DEG2RAD(rr));
  }
  

  /* map each component into the Hough accumulator space */
  for(y=0;y<image->h;y++)
    for(x=0;x<image->w;x++) 
      if(pbm_getpixel(image->bitmap,x,y)){
	
	/* for each component, draw the sinusoid in Hough space */
	for (omega = hs; omega < hf; omega++){
	  
	  /* centre_y-yd because +ve skew is anticlockwise from east */
	  rr = (centre_y-y)*sin_values[omega-hs] + (x-centre_x)*cos_values[omega-hs];
	  
	  /* div_amount is the scale of r */
	  rr=rr/div_amount;
	  
	  /* clamp into the Hough space */
	  r=rmax+ROUND(rr);
	  if(r<0) {
	    r=0;
	  } else {
	    if(r>=2*rmax+1) {
	      r=2*rmax+1-1;
	    }
	  }
	  
	  pos=&accum[r][omega-hs];
	  
	  /* update the accumulator with log(number of pixels) */
	  (*pos)+=increment;
	  
	  if((*pos)>maxaccum){
	    maxaccum=(*pos);
	  }
	}
      }

  /* Write out original accumulator array */
  CALLOC_2D(save_pgm,2*rmax+1, hf-hs+1,double); 
  
  for(r=0;r<2*rmax+1;r++)
    for(omega=hs; omega<hf;omega++)
      save_pgm[r][omega-hs]=accum[r][omega-hs];

  /* store as a P2 with proper maximum so we know the original range */
  {
    char commentstring[1000];
    sprintf(commentstring,"Pixels: HS %g, HF %g, NUM_BINS %g, r_div %d\n",HS,HF,NUM_BINS,div_amount);
    write_window_2D_pgm_P2(raw_filename,save_pgm,0,2*rmax,0,hf-hs-1,ROUND(maxaccum),commentstring);
  }
  FREE_2D(save_pgm,2*rmax+1);
  
  da_free(&ddd);

  FREE_2D(accum,2*rmax+1);
}


/* calls hough_calc_range with binsize instead of number of bins */
void
hough_calc(marklistptr list, 
	   int w, 
	   int h, 
	   double low,
	   double high,
	   double binsize, 
	   int    rdiv,
	   char accum_name[],
	   int uselog)
{
  double numbins;

  numbins=ROUND((high-low+1)/binsize); /* number of bins */
  if(numbins<=1) numbins=1;

  hough_calc_range(list,w,h,low,high,numbins,rdiv,accum_name,uselog);
}


/* calls hough_calc_range with binsize instead of number of bins */
void
hough_image_calc(marktype *image,
		 double low,
		 double high,
		 double binsize, 
		 int    rdiv,
		 char accum_name[])
{
  double numbins;

  numbins=ROUND((high-low+1)/binsize); /* number of bins */
  if(numbins<=1) numbins=1;

  hough_image_calc_range(image,low,high,numbins,rdiv,accum_name);
}
