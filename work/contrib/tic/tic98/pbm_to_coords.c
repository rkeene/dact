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
	         "\tpbm_to_coords pbm > coords \n");
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


  if(argc<2) usage();

  list=NULL;

  infile=argv[1];
  if(infile==NULL)
    exit(1);

  if(marktype_readnamed(infile,&image)!=0)
    exit(1);
  marktype_area(&image);

  list=extract_all_marks(list,image,nested,conn);

  /*fprintf(stdout,"%d %d %d # num xcen l r ycen t b  area\n",image.w,image.h,marklist_length(list));*/

  for(step=list ; step ; step=step->next) {
    fprintf(stdout,"%d %d %d %d %d %d %d   %d\n",
	    step->data.symnum,
	    step->data.xpos+step->data.xcen,
	    step->data.xpos,
	    step->data.xpos+step->data.w-1,
	    step->data.ypos+step->data.ycen,
	    step->data.ypos,
	    step->data.ypos+step->data.h-1,
	    step->data.set
	    );

  }
  marklist_free(&list);
  marktype_free(&image);
  
  exit(0);
}
