// 	%Z% %Y% %M% version %I%, %G%

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>


#include "marklist.h"
#include "pbmtools.h"

#include <math.h>



void usage()
{
  fprintf(stderr,"usage: \n\tcalc_hough  [options]  <image>\n" 
	  "options:\n"
	  " -f n\tsets the raw filename\n"
	  "\n"
	  "note:\n"
	  "tThe raw_data and filtered files are only written after the 1st iteration.\n"
	  );
  exit(1);
}


int main(int argc, char *argv[])
{
    int i,ch;
    int w,h;
    int pixels;
    marktype image;    
    struct timeval tv;
    int test=0;

    gettimeofday(&tv, NULL);

    srandom(tv.tv_usec);

    while((ch = getopt(argc, argv, "pvb:r:f:l:h:")) != -1)
      switch(ch){
      }
    
    w=(random()%400);
    h=(random()%300);

    test=random()%10;
    if(test==0){
      w=0;
    } else if(test==1){
      h=0;
    } else if(test==2){
      w=h=0;
    } else {
    }


    test=random()%10;
    if(test==0){
      pixels=0;
    } else if(test==1){
      pixels=w*h;
    } else {
      pixels=(int)( ((random()%(w*h+1))));
    }

    marktype_alloc(&image,w,h);
    fprintf(stderr,"random: %dx%d, %d pixels\n",w,h,pixels);

    for(i=0;i<pixels;i++){
      int xx=random()%w,yy=random()%h;
      ppix(image,xx,yy,1);
    }

    marktype_write(stdout,image);
    marktype_free(&image);
    

    return 0;
}
