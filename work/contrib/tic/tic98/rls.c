#include <string.h>

#include <stdio.h>

#include "boundary.h"
#include "rls.h"
#include "marklist.h"

#include <assert.h>

marktype rls_smoothed_x(const marktype image, int tx)
{
  marktype smoothed;
  int x, y, t;
  int next;

  smoothed=marktype_copy(image);
    
  for (y = 0 ; y < image.h; y++){
    for (x = 0 ; x < image.w; x++){
      if(gpix(smoothed,x,y)){
	for(t=x+1;t<image.w;t++)
	  if(gpix(smoothed,t,y)==1)
	    break;
	if((t-x<=tx) && (t<image.w)){
	  for(next=x+1;next<t;next++)
	    ppix(smoothed,next,y,1);
	}
	x=t-1;
      }
    }
  }
  return smoothed;
}


marktype rls_smoothed_y(const marktype image, int ty)
{
  marktype smoothed;
  int x, y, t;
  int next;

  smoothed=marktype_copy(image);
    
  for (x = 0 ; x < image.w; x++){
    for (y = 0 ; y < image.h; y++){
      if(gpix(smoothed,x,y)){
	for(t=y+1;t<image.h;t++)
	  if(gpix(smoothed,x,t)==1)
	    break;
	if((t-y<=ty) && (t<image.h)){
	  for(next=y+1;next<t;next++)
	    ppix(smoothed,x,next,1);
	}
	y=t-1;
      }
    }
  }
  return smoothed;
}


marktype rls(const marktype image, int tx, int ty)
{
  marktype smoothed,image_x,image_y;
  int x, y, t,p1=1,p2=1;
  int next;

  image_x=rls_smoothed_x(image,tx);
  image_y=rls_smoothed_y(image,ty);

  marktype_alloc(&smoothed,image_x.w,image_x.h);
  
  for (x = 0 ; x < image_x.w; x++){
    for (y = 0 ; y < image_x.h; y++){
      p1=gpix(image_x,x,y);
      p2=gpix(image_y,x,y);
      
      if((p1==1) || (p2==1))
	ppix(smoothed,x,y,1);
    }
  }
  
  marktype_free(&image_x);
  marktype_free(&image_y);
  return smoothed;
}







void usage()
{
    fprintf(stderr,"usage: rls image.pbm zonefile\n");
    exit(1);
}

int main(int argc, char *argv[])
{
  marktype image,image_rls;
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

  image_rls=rls(image,40,40);

  marktype_writenamed("out.pbm",image_rls);

  list=extract_all_marks(list,image_rls,0,4);

  for(i=0,step=list;step;step=step->next){
    marktype *temp=&(step->data);

    istext=0;
    isnot=0;
    for(tt=0;tt<num_tags;tt++){
      if((temp->xpos+temp->xcen>=_x1[tt]) && 
	 (temp->xpos+temp->xcen<_x2[tt]) &&
	 (temp->ypos+temp->ycen>=_y1[tt]) &&
	 (temp->ypos+temp->ycen<_y2[tt])){
	if(_tags[tt]==0){
	  istext++;
	} else {
	  isnot++;
	}
      }
    }

    if((istext>isnot) && (istext>=1)){
      sprintf(tag,"text");
    } else if((isnot>istext) && (isnot>=1)){
      sprintf(tag,"drawing");
    } else {
      sprintf(tag,"?");
    }
      
    if(temp->set*1.0/(temp->w*temp->h) > 0.25){
      fprintf(stdout,"%03d %s %d %d %d %d\n",i,tag,temp->xpos,temp->ypos,temp->xpos+temp->w,temp->ypos+temp->h);
      
      i++;
    }
  }
    

  marktype_free(&image);
  marktype_free(&image_rls);

  return 0;

}

