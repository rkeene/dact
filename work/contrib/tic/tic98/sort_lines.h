#ifndef _SORT_LINES_H
#define _SORT_LINES_H

#include "marklist.h"

marklistptr sortmarks_profile(marklistptr listofmarks, 
		      const int w, 
		      const int h,
		      const double rotated_angle, 
		      const int DPI);

marklistptr sortmarks_random(const marklistptr listofmarks, 
		      const int imagew,
		      const int imageh,
		      const double rotated_angle, 
		      const int DPI);


marklistptr sortmarks_zhang(const marklistptr listofmarks, 
		      const int imagew,
		      const int imageh,
		      const double rotated_angle, 
		      const int DPI);

marklistptr sortmarks_howard(const marklistptr listofmarks, 
		      const int imagew,
		      const int imageh,
		      const double rotated_angle, 
		      const int DPI);

void rot(double *x, double *y, const double theta, 
	 const double cx, const double cy);
#endif

