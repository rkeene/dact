#include <string.h>

#include <stdio.h>

#include "boundary.h"
#include "rls.h"
#include "marklist.h"

#include <assert.h>




void
marktype_box(marktype image, int x1, int y1, int x2, int y2)
{
    marktype_line(image,x1,y1,x2,y1);
    marktype_line(image,x1,y2,x2,y2);
    marktype_line(image,x1,y1,x1,y2);
    marktype_line(image,x2,y1,x2,y2);
}

void usage()
{
    fprintf(stderr,"usage: zone_highlight image.pbm zonefile\n");
    exit(1);
}

int main(int argc, char *argv[])
{
  marktype image;
  marklistptr list=NULL,step;
  double bestaccum,accum;
  double angle;
  int len,i,r;
  int _x1[1000],_y1[1000],_x2[1000],_y2[1000],_tags[1000];
  char s[100],tag[100];
  FILE *fp=NULL;
  int num_tags=0;
  int x1,y1,x2,y2,tt;
  int istext=0, isnot=0;
  
  if(argc<3) {
    usage();
  }

  fp=fopen(argv[2],"rt");
  if(fp){
    while(fgets(s,100,fp)){
	
      sscanf(s,"%*s %s %d %d %d %d\n",tag,
	     &_x1[num_tags],
	     &_y1[num_tags],
	     &_x2[num_tags],
	     &_y2[num_tags]);
      if(strncmp(tag,"text",4)==0)
	_tags[num_tags]=0;
      else
	_tags[num_tags]=1;

      num_tags++;
      assert(num_tags<1000);
    }
  } else {
    fprintf(stderr,"error: can't open file: %s\n",argv[2]);
    exit(1);
  }
  


  if(marktype_readnamed(argv[1],&image)!=0){
    fprintf(stderr,"error loading: %s\n",argv[1]);
    return 0.0;
  }

  if(argc>=2)
    r=atoi(argv[1]);
  else
    r=0;

  
  for(tt=0;tt<num_tags;tt++){
    marktype_box(image,_x1[tt],_y1[tt],_x2[tt],_y2[tt]);
    marktype_box(image,_x1[tt]-1,_y1[tt]-1,_x2[tt]+1,_y2[tt]+1);
    marktype_box(image,_x1[tt]-2,_y1[tt]-2,_x2[tt]+2,_y2[tt]+2);
    marktype_box(image,_x1[tt]-3,_y1[tt]-3,_x2[tt]+3,_y2[tt]+3);
    marktype_box(image,_x1[tt]-4,_y1[tt]-4,_x2[tt]+4,_y2[tt]+4);
    marktype_box(image,_x1[tt]-5,_y1[tt]-5,_x2[tt]+5,_y2[tt]+5);
  }

  
  marktype_write(stdout,image);

  marktype_free(&image);

  return 0;

}

