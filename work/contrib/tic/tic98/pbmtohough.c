// 	%Z% %Y% %M% version %I%, %G%

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"
#include "marklist.h"
#include "pbmtools.h"
#include "boundary.h"
#include "timer.h"

#include "hough.h"
#include <math.h>

marktype image;
int VV=0;


void 
calc_skew(double low, 
	  double high,
	  double binsize, 
	  int rdiv, 
	  char *rawfilename,
	  int pixels,
	  int uselog)
{
  if(pixels==0){
    marktype bitmapcopy;
    marklistptr list=NULL;
    
    bitmapcopy=marktype_copy(image);
    list=extract_all_marks(list,image,1,8);
    hough_calc(list,image.w,image.h,low,high,binsize, rdiv,rawfilename,uselog);
  } else {
    hough_image_calc(&image,low,high,binsize, rdiv,rawfilename);
  }
      
}


void usage()
{
  fprintf(stderr,"usage: \n\tcalc_hough  [options]  <image>\n" 
	  "options:\n"
	  "  -p\tUse pixels instead of components\n"
	  "  -L\tUse the log(component area) as the accumulator increment\n"
	  "  -l n\tSets the low bound of the Hough transform to <n> (-90)\n"
	  "  -h n\tSets the high bound of the Hough transform to <n> (90)\n"
	  "  -b n\tSets the bin size to <n> (1)\n"
	  "  -r n\tSets the r divisor to <n> (1)\n"
	  "\n"
	  );
  exit(1);
}


int main(int argc, char *argv[])
{
    int ch;
    char *imagefilename=NULL;
    int rdiv=1;
    double low=-90,high=90;
    double binsize=1;
    int pixels=0;
    char *rawfilename=NULL;
    int uselog=0;
    
    if(argc<2) usage();

    while((ch = getopt(argc, argv, "pvb:r:f:l:h:L")) != -1)
      switch(ch){
      case 'p':    pixels=1;break;
      case 'b':    binsize=atof(optarg); break;
      case 'r':    rdiv=atoi(optarg); break;
      case 'f':    rawfilename=optarg; break;     
      case 'L':    uselog=1; break;     
      case 'l':    low=atof(optarg); break;     
      case 'h':    high=atof(optarg); break;     
      case '?':
	usage();
      }
    
    for(; optind <argc; optind++){
	if(imagefilename==NULL) imagefilename=strdup(argv[optind]);
	else error(argv[0],"too many filenames","");
    }

    if(imagefilename==NULL){
      usage();
    }

    if(marktype_readnamed(imagefilename,&image)){
      fprintf(stderr,"error - can't open file %s\n",imagefilename);
      exit(1);
    }

    calc_skew(low,high,binsize,rdiv,rawfilename,pixels,uselog);

    return 0;
}
