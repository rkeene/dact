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
  float a[7];
  
  x_col--;
  y_col--;
  assert(x_col>=0 && x_col<=6);
  assert(y_col>=0 && y_col<=6);

  scanf("%d %d %d\n",&width,&height,&num);

  if(num){
    x_ary=(float*)calloc(num,sizeof(float));
    assert(x_ary);
    y_ary=(float*)calloc(num,sizeof(float));
    assert(y_ary);
  }
  for(i=0;i<num;i++){
    err=scanf("%f %f %f %f %f %f %f\n",&a[0],&a[1],&a[2],&a[3],&a[4],&a[5],&a[6]);
    assert(err==7);
    x_ary[i]=a[x_col];
    y_ary[i]=a[y_col];
    printf("%f %f\n",x_ary[i],y_ary[i]);
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

  if(argc<3) usage();

  read_coord_file(atoi(argv[1]), atoi(argv[2]));



  exit(0);
}
