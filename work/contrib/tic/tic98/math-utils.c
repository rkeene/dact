/*****************************************************************************
 *
 *   Author:   Stuart Inglis     <singlis@waikato.ac.nz>
 *
 *
 * This file contains functions which are common utilities
 *
 *****************************************************************************/
#include <stdio.h>
#include <math.h>

#include "marklist.h"
#include "pbmtools.h"

#include "utils.h"
#include "math-utils.h"



double pow_0(double p, double q)
{
  if((p==0.0)&&(q==0.0))
    return 0;
  else return pow(p,q);
}


void setdp(float *f, int dp)
{
  int l,i;
  double scale;

  scale=1.0;
  for(i=0;i<dp;i++)
      scale*=10;
/*  scale=exp10((double)dp);*/
  
  l=(int)((*f)*scale+0.5);
  *f=(float)(l/scale);
}




/* 
   returns between -pi and +pi, ie. [-180 to +180)

                  | +pi/2 (90)
                  |
                  |
                  |
-180   -----------+-------------  0 
(-pi)             |
                  |
                  |
                  | -pi/2 (-90)

to normalise between [-90, 90),
  if(ang>90) ang=ang-180;
  else if (ang<-90) ang=ang+180;
*/
double atan_xy(double yd, double xd)
{
    double angle;
    
    angle=atan2(yd,xd);  /* returns a result between (-PI, PI] */

    while(angle>=PI) 
	angle=angle-2*PI;/* now returns a result between [-PI, PI) */
 
/*    assert((angle>=-PI) && (angle<PI));*/

    return angle;
}
