/*****************************************************************************
 *
 *   Author:   Stuart Inglis     <singlis@waikato.ac.nz>
 *
 *
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "getopt.h"
#include "marklist.h"
#include "pbmtools.h"
#include "boundary.h"
#include "utils.h"

#ifndef PI
#define PI 3.14159265
#endif

void rot(double *x, double *y, const double theta, 
	 const double cx, const double cy)
{
  double xx,yy;
  *x-=cx;
  *y-=cy;
  xx=(*x)*cos(theta) + (*y)*sin(theta);
  yy=-(*x)*sin(theta) + (*y)*cos(theta);
  *x=xx+cx;
  *y=yy+cy;
}



void 
usage()
{
  fprintf(stderr,
	  "usage:\n"
	  "\tcoords_profile x y < coords > profile \n"
	  "\n"
	  "\tThe coordinates are stored in two columns. The first\n"
	  "\tline of the file contains the width/height of the image\n"
	  "\tand the number of components. ie. <w> <h> <n>\n"
	  );
  
  exit(1);
}

int width=0,height=0,num=0;
float *x_ary=NULL;
float *y_ary=NULL;

int
read_coord_file(int x_col, int y_col)
{
  int i,err;
  float a[8];
  char line[100];
  
  x_col--;
  y_col--;
  assert(x_col>=0 && x_col<=7);
  assert(y_col>=0 && y_col<=7);

  fgets(line,100,stdin);
  sscanf(line,"%d %d %d\n",&width,&height,&num);

  if(num){
    x_ary=(float*)calloc(num,sizeof(float));
    assert(x_ary);
    y_ary=(float*)calloc(num,sizeof(float));
    assert(y_ary);
  }
  for(i=0;i<num;i++){
    err=scanf("%f %f %f %f %f %f %f %f\n",&a[0],&a[1],&a[2],&a[3],&a[4],&a[5],&a[6],&a[7]);
    assert(err==8);
    x_ary[i]=a[x_col];
    y_ary[i]=a[y_col];
  }
  
}


int
main(int argc, char *argv[])
{
  int ch;
  marklistptr list=NULL,step=NULL;
  marktype image;
  char *infile=NULL;
  int conn=8;
  int nested=1;
  double a,x,y;
  double tot;
  int i,p;
  int hist[10000];

  if(argc<3) usage();

  read_coord_file(atoi(argv[1]), atoi(argv[2]));

  for(a=-10 ; a<=10 ; a+=0.1){
    for(i=0;i<10000;i++) 
      hist[i]=0;
    
    for(i=0;i<num;i++){
      x=x_ary[i];
      /* the origin is at the top */
      y=height-1-y_ary[i];
      rot(&x,&y,DEG2RAD(a),width/2.0,height/2.0);

      p=height+(int)(y);
      assert(p>=0 && p<10000);
      hist[p]++;
    }

    tot=0;
    for(i=0;i<10000;i++){
      tot += (hist[i]*1.0 * hist[i]);
    }
    fprintf(stdout,"%f %f\n",a,tot);
  }

  exit(0);
}
