/*
 * Author:   
 *    Stuart Inglis (singlis@internz.co.nz)
 *    (c) 1994-1998
 * 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"

#include "boundary.h"

void
calc_features(marklistptr *list)
{
  int len=0;
  float area=0;
  float avg=0,avg_t=0;
  float density=0,density_t=0;
  float aspect=0,aspect_t=0;
  float holes=0,holes_t=0;
  marklistptr step=NULL;

  len=marklist_length(*list);
  for(step=*list; step; step=step->next){
    area+=step->data.set;
  }

  for(step=*list; step; step=step->next){
    density=(step->data.set*1.0/(step->data.w*step->data.h));
    density_t+=density;
  }
  density=density_t/len;

  for(step=*list; step; step=step->next){
    aspect=(step->data.w*1.0/step->data.h);
    aspect_t+=aspect;
  }
  aspect=aspect_t/len;

  for(step=*list; step; step=step->next){
    marktype copy;
    marklistptr list2=NULL;
    int i,j;
    int w,h;

    w=step->data.w+2;
    h=step->data.h+2;
    
    marktype_alloc(&copy,w,h);
    marktype_placeat(copy,step->data,1,1);
    
    for(j=0;j<copy.h;j++)
      for(i=0;i<copy.w;i++){
	int p=gpix(copy,i,j);
	ppix(copy,i,j,!p);
      }
    
    list2=extract_all_marks(list2,copy,1,8);

    holes=marklist_length(list2)-1; if(holes<0) holes=0;

    holes_t+=holes;
    marklist_free(&list2);
    marktype_free(&copy);
  }
  holes=holes_t/len;
  
  fprintf(stderr,"%d  %.1f  %.1f  %.2f  %.2f  %.2f\n",len,area,area/len,density,aspect,holes);

}

marklistptr 
imagefn_to_list(char *fn, int *w, int *h)
{
  marktype image;
  marklistptr list=NULL,list2=NULL;

  if(marktype_readnamed(fn, &image)!=0)
    return NULL;

  *w=image.w;
  *h=image.h;

  if(image.resolution==0){
    image.resolution=300;
  }
  
  /* rotate the image, keeping original size */
  list=extract_all_marks(list,image, 1,4);

  marklist_prune_under_area(&list,0.2,image.resolution);
  
  marklist_prune_over_area(&list,4,image.resolution);
  
  if(0){
    marktype im;
    marklist_reconstruct_show_lines (list, &im);
    marktype_writenamed("-",im);
    marktype_free(&im);
  }

  marktype_free(&image);
  return list;
}


int 
main(int argc, char *argv[])
{
  int ch;
  extern char *optarg;
  int compress=1;
  int num_in=0;
  int num_out=0;
  char **infilename=NULL;
  char **outfilename=NULL;
  int w,h;
  marklistptr list=NULL;
  int i,j;
  char s[100];
  

  if(argc<2)
    exit(1);

  for(i=1;i<argc;i++){
    list=imagefn_to_list(argv[i],&w,&h);

    strcpy(s,argv[i]);
    for(j=0;j<strlen(s);j++)
      if(s[j]=='.') s[j]=0;
    fprintf(stderr,"%s  ",s);
        
    calc_features(&list);
    
    marklist_free(&list);
  }
  exit(0);
}
