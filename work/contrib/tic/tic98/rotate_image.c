
#include <stdio.h>
#include <math.h>

#include "marklist.h"
#include "rotate.h"


void usage(void)
{
    fprintf(stderr,
	    "usage: rotate_image [options] <filename> <skew>\n"
	    "\n"
	    "options:\n"
	    "  -a\tthe angle is specified in radians\n"
	    "  -k\tkeep original file size\n"
	    "\n"
	    "notes:\n"
	    "\ta positive skew, is a clockwise rotation from\n"
	    "\tdue east. ie. 4.75 degrees\n"
	    );
    exit(1);
}
    

int main(int argc, char *argv[])
{
    marktype image;
    double angl=-999;
    char *infile=NULL;
    extern char *optarg;
    extern int optind;
    int keep_orig_size=0;
    int ch;
    int radians=0;

    while((ch = getopt(argc, argv, "kr")) != -1)
      switch(ch){
      case 'k': keep_orig_size=1;break;
      case 'r': radians=1; break;
      default:
	usage();
	break;
      }
    
    for(; optind <argc; optind++){
	if(!infile) infile=argv[optind];
	else if(angl==-999) angl=atof(argv[optind]);
	else usage();
    }

    if((infile==NULL) || (angl==-999))
      usage();
    
    marktype_readnamed(infile,&image);

    if(radians==0){
      angl=DEG2RAD(angl);
    }
    rotate_image(&image,angl,keep_orig_size);

    marktype_writenamed("-",image);
    return 0;
}
