
/*****************************************************************************
 *
 *   Author:   Stuart Inglis     <singlis@waikato.ac.nz>
 *
 *
 * This file contains routines dealing with portable bitmap files (PBM)
 *
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "marklist.h"
#include "utils.h"
#include "rotate.h"


/* a positive rotation angle is anti-clockwise */
void rotate_image(marktype *image,double angle, int keep_orig_size)
{
  marktype rotimage;
  int nx,ny,newheight,newwidth,oldheight,oldwidth,i,j,halfnewheight,halfnewwidth;
  int halfoldheight,halfoldwidth;
  double radians; 
  double cosval,sinval;
  int pix;

  if(angle == 0)
    return;

  angle=-angle;

  radians =  angle;
  cosval = cos(radians);
  sinval = sin(radians);
  oldheight = image->h;
  oldwidth = image->w;

  if(keep_orig_size){
    newwidth=oldwidth;
    newheight=oldheight;
  } else {
    newwidth = abs((int)(oldwidth*cosval)) + (int)abs((int)(oldheight*sinval));
    newheight = abs((int)(-oldwidth*sinval)) + (int)abs((int)(oldheight*cosval));
  }
  
  halfnewheight = newheight / 2;
  halfnewwidth = newwidth / 2;
  halfoldwidth = oldwidth /2;
  halfoldheight = oldheight /2 ;

  marktype_alloc(&rotimage,newwidth,newheight);

  rotimage.resolution=image->resolution;
  
  for(i=0;i < newheight;i++){
    for(j=0;j < newwidth;j++){
      /*      pbm_putpixel(rotimage.bitmap,j,i,0);*/

      nx = (int)( (j - halfnewwidth)*cosval + (i-halfnewheight)*sinval+0.5);
      ny = (int)( (0-(j - halfnewwidth))*sinval + (i - halfnewheight)*cosval+0.5);
      nx = nx + halfoldwidth;
      ny = ny + halfoldheight;
      pix=gpix((*image),nx,ny);
      ppix(rotimage,j,i,pix);
      /*      if ((nx < oldwidth) && (ny < oldheight) && (nx >= 0) && (ny >= 0))
	      pbm_putpixel(rotimage.bitmap,j,i,pbm_getpixel(image->bitmap, nx, ny));*/
    }
  }
  marktype_free(image);
  *image=rotimage;
}



static int **newimage(int cols, int rows)
{
  int **p;
  int i;
  
  p=(int**)malloc(rows*sizeof(int*));
  if(p==NULL) error("newimage","out of memory","");
  for(i=0;i<rows;i++){
    p[i]=(int*)calloc(cols,sizeof(int));
    if(p[i]==NULL) error("newimage","out of memory","");
  }
  return p;
}


static void freeimage(int ***image,int rows)
{
  int r;
  for(r=0;r<rows;r++)
		free((*image)[r]);
  free(*image);
  (*image)=NULL;
}


#define SCALE 10

void r_hough(marktype image, int threshold)
{
  int **z;
  int center_x, center_y, r, omega, i, j, rmax;
  double rr;
  double sarr[180*SCALE], carr[180*SCALE];
  
  center_x = image.w/2; center_y = image.h/2;
  rmax = (int)(sqrt((double)(image.w * image.w + image.h * image.h))/2.0);
  
  z = newimage(180*SCALE, 2*rmax + 1);
  
  for (omega = 0; omega < 180*SCALE; omega++){
      rr = ((double)omega)/(double)SCALE;
      sarr[omega] = sin((double)DEG2RAD(rr));
      carr[omega] = cos((double)DEG2RAD(rr));
    }
  
  /* initialise z */
  for (r = 0; r < 2 * rmax + 1; r++)
    for (omega = 0; omega < 180*SCALE; omega++)
      z[omega][r] = 0;
  
  for (i = 0; i < image.w; i++)
    for (j = 0; j < image.h; j++){
      if (pbm_getpixel(image.bitmap,i,j))
	for (omega = 0; omega < 180*SCALE; ++omega){
	  rr = (j - center_y) * sarr[omega]
      	     - (i - center_x) * carr[omega];
	  if (rr < 0.0) r = (int)rr;
	  else r = (int)rr + 1;
	  z[omega][rmax + r] += 1;
	}
    }

  
  freeimage(&z,2*rmax+1);
}
