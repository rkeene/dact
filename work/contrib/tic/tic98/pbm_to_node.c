/*****************************************************************************
 *
 *   Author:   Stuart Inglis     <singlis@waikato.ac.nz>
 *
 *
 * Extracts the coordinates of components from an image. The origin 
 * is the lower left corner.
 *
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>

#include "marklist.h"
#include "pbmtools.h"
#include "boundary.h"
#include "utils.h"


void 
usage()
{
  fprintf(stderr,"usage:\n"
	         "\tpbm_to_node pbm > nodes \n");
  exit(1);
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

  extern char *optarg;
  extern int optind;
  int len,i,j;
  int *xx,*yy;


  if(argc<2) usage();

  list=NULL;

  infile=argv[1];
  if(infile==NULL)
    exit(1);

  if(marktype_readnamed(infile,&image)!=0)
    exit(1);
  marktype_area(&image);

  list=extract_all_marks(list,image,nested,conn);
  len=marklist_length(list);

  CALLOC(xx,len,int);
  CALLOC(yy,len,int);

  fprintf(stdout,"%d %d\n",len,len*(len-1));

  for(i=0,step=list;step;i++,step=step->next){
    xx[i]=step->data.xpos+step->data.xcen;
    yy[i]=step->data.ypos+step->data.ycen;
  }

  for(i=0;i<len;i++){
    if(!(i%10)){fprintf(stderr,"%d/%d\r",i,len-1);fflush(stderr);}
    for(j=0;j<len;j++)if(i!=j){
      float xd,yd,d;

      xd=xx[i] - xx[j];
      yd=yy[i] - yy[j];
      d=xd*xd + yd*yd;
      /*if(d<0)d=0;
      d=sqrt(d);*/
      
      printf("%d %d %d\n",i,j,(int)d);
	
    }
  }
  marklist_free(&list);
  marktype_free(&image);
  
  exit(0);
}
