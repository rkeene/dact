#ifndef __HOUGH_H
#define __HOUGH_H

#include "marklist.h"

void
hough_calc(marklistptr list, 
	   int image_w, 
	   int image_h, 
	   double low,
	   double high,
	   double binsize, 
	   int    rdiv,
	   char accum_name[],
	   int uselog);

void
hough_image_calc(marktype *image,
		 double low,
		 double high,
		 double binsize, 
		 int    rdiv,
		 char accum_name[]);

#endif
