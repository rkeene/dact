
/*****************************************************************************
 *
 *   Author:   Stuart Inglis     <singlis@waikato.ac.nz>
 *
 *
 *
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <assert.h>

#include "marklist.h"
#include "vector.h"
#include "windowing.h"
#include "utils.h"
#include <assert.h>



void 
vector_init(vectortype *v)
{
  v->size=0;
  v->v=NULL;
}


void 
vector_alloc(vectortype *v, int size)
{
  vector_init(v);
  CALLOC(v->v,size,double);
  v->size=size;
}

void 
vector_free(vectortype *v)
{
  if(v->v)
    FREE(v->v);
}


void 
vector_fprintf(FILE* fp, vectortype *hist)
{
  if(hist->v){
    int i;
    for(i=0;i<hist->size;i++)
      fprintf(fp,"%f\n",hist->v[i]);
  }
}

void 
vector_fprint_named(char *fn, vectortype *hist)
{
  FILE *fp;

  fp=fopen(fn,"wt");
  if(fp==NULL){
    fprintf(stderr,"error: vector_fprint_named() can't write to %s\n",fn);
  } else {
    vector_fprintf(fp,hist);
    fclose(fp);
  }
}



void 
vector_image_vertical_rles(vectortype *hist, marktype image)
{
  int rl,p,p2;
  int max=0;
  int y,x;
  int count;

  assert(hist->size>=image.w);
  
  for(x=0;x<image.w;x++){
    count=0;
    rl=0;
    y=0;
    p=gpix(image,x,y);
    while(y<image.h){
      p2=gpix(image,x,y);
      if((p2==0) && (p2==p))
	rl++;
      else{
	if(rl)
	  count+=(rl*rl);
	rl=0;
      }
      p=p2;
      y++;
    }
    if(rl) count+=(rl*rl);

    hist->v[x]=count;
    if(count>max){
      max=(int)count;
    }
  }
  for(x=0;x<hist->size;x++){
    hist->v[x]/=max;
  }
}



void 
vector_image_vertical(vectortype *hist, marktype image)
{
  int y,x,count,p;

  assert(hist->size>=image.w);
  
  for(x=0;x<image.w;x++){
    count=0;
    for(y=0;y<image.h;y++){
      p=gpix(image,x,y);
      count+=p;
    }
    hist->v[x]=count;
  }
}

void 
vector_image_horizontal(vectortype *hist, marktype image)
{
  int y,x,count,p;

  assert(hist->size>=image.h);
  
  for(y=0;y<image.h;y++){
    count=0;
    for(x=0;x<image.w;x++){
      p=gpix(image,x,y);
      count+=p;
    }
    hist->v[y]=count;
  }
}



void 
vector_image_horizontal_rles(vectortype *hist, marktype image)
{
  int rl,p,p2;
  int max=0;
  int y;
  int count;

  assert(hist->size>=image.h);
  
  for(y=0;y<image.h;y++){
    int x=0;
    count=0;
    rl=0;
    y=0;
    p=gpix(image,x,y);
    while(x<image.w){
      p2=gpix(image,x,y);
      if((p2==0) && (p2==p))
	rl++;
      else{
	if(rl)
	  count+=(rl*rl);
	rl=0;
      }
      p=p2;
      x++;
    }
    if(rl) count+=(rl*rl);

    hist->v[y]=count;
    if(count>max){
      max=(int)count;
    }
  }
  for(y=0;y<hist->size;y++){
    hist->v[y]/=max;
  }
}


void 
vector_norm(vectortype *hist)
{
  double max=0;
  int x;

  if(hist->size){
    max=hist->v[0];
    for(x=0;x<hist->size;x++){
      if(hist->v[x]>max)
	max=hist->v[x];
    }
    if(max){
      for(x=0;x<hist->size;x++)
	hist->v[x]/=max;
    }
  }
}


void 
vector_invert(vectortype *hist)
{
  double max=0;
  int x;

  if(hist->size){
    max=hist->v[0];
    for(x=0;x<hist->size;x++){
      if(hist->v[x]>max)
	max=hist->v[x];
    }
    for(x=0;x<hist->size;x++)
      hist->v[x]=max-hist->v[x];
  }
}


void 
vector_quantise(vectortype *hist, int levels)
{
  int x;
  
  if(hist->size && levels>0){
    vector_norm(hist);
    for(x=0;x<hist->size;x++){
      hist->v[x]*=(levels);

      hist->v[x]=ROUND(hist->v[x]);
    }
  }
  vector_norm(hist);
}

void 
vector_threshold(vectortype *hist, double thres, double below, double above)
{
  int x;
  
  if(hist->size){
    vector_norm(hist);
    for(x=0;x<hist->size;x++){
      if(hist->v[x]<=thres)
	hist->v[x]=below;
      else
	hist->v[x]=above;
    }
  }
}


void 
vector_convolve(vectortype *hist, int size, int TYPE)
{
  vectortype result;
  double *window;
  int x;

  if(size>0){
    vector_alloc(&result,hist->size);
    
    if((size%2)==0) size++;

    CALLOC(window,size,double);
    
    create_window(window, size,TYPE);
    convolve(hist->v,0,hist->size-1, window, size,size/2,result.v,0);
    
    for(x=0;x<hist->size;x++)
      hist->v[x]=result.v[x];
    FREE(window);
    vector_free(&result);
  }
  
  vector_norm(hist);
}




void 
vector_image_highlight_vertical(vectortype *hist, marktype image)
{
  int x;
  if(hist->size){
    for(x=0;x<image.w;x++){
      int p;
      int y;
      double r;

      for(y=0;y<image.h;y++){

	r=(rand()%10001)/10000.0;

	if(hist->v[x]>=r){
	  p=gpix(image,x,y);
	  ppix(image,x,y,!p);
	}
      }
    }
  }
}

void 
vector_image_highlight_horizontal(vectortype *hist, marktype image)
{
  int y;
  if(hist->size){
    for(y=0;y<image.h;y++){
      int p;
      int x;
      double r;

      for(x=0;x<image.w;x++){

	r=(rand()%10001)/10000.0;

	if(hist->v[y]>=r){
	  p=gpix(image,x,y);
	  ppix(image,x,y,!p);
	}
      }
    }
  }
}


void
vector_image_plot(vectortype *hist, marktype image)
{
  if(hist->size){
    int y;
    int x;
    vector_norm(hist);
    for(x=0;x<image.w;x++){
      int p;

      p=ROUND( ( 1-hist->v[x]) * (image.h-1));
      for(y=image.h-1;y>=p;y--)
	ppix(image,x,y,1);
      for(;y>=0;y--)
	ppix(image,x,y,0);
    }
  }
}


void 
vector_and(vectortype *res, vectortype *anded)
{
  int i;

  assert(res->size==anded->size);

  for(i=0;i<res->size;i++)
    res->v[i]=res->v[i] * anded->v[i];

  vector_norm(res);
}

void 
vector_or(vectortype *res, vectortype *ored)
{
  int i;

  assert(res->size==ored->size);

  for(i=0;i<res->size;i++)
    res->v[i]=res->v[i] + ored->v[i];

  vector_norm(res);
}


void 
vector_mean(vectortype *hist1)
{
  vectortype result;
  int i,j;
  int mean;
  
  vector_alloc(&result,hist1->size);

  for(i=0;i<hist1->size;i++){
    if((hist1->v[i]>=0.99)){
      for(j=i+1;j<hist1->size;j++)
	if(hist1->v[j]<0.99)
	  break;
      mean=ROUND((j-1+i)/2.0);
      result.v[mean]=1.0;
      i=j;
    }
  }
  vector_norm(&result);
  for(i=0;i<hist1->size;i++){
    hist1->v[i]=result.v[i];
  }
  vector_free(&result);
}



void 
vector_edge(vectortype *hist1)
{
  vectortype result;
  int i;
  
  vector_alloc(&result,hist1->size);

  for(i=1;i<hist1->size;i++){
    if(fabs(hist1->v[i]-hist1->v[i-1])>=0.99)
      result.v[i]=1.0;
    else
      result.v[i]=0.0;
  }
  for(i=0;i<hist1->size;i++){
    hist1->v[i]=result.v[i];
  }
  vector_free(&result);
}

void 
vector_stats(vectortype *hist, double *mean, double *sd)
{
  double sumx=0,sumxx=0;
  int n=0,x;

  assert(mean);
  assert(sd);

  for(x=0;x<hist->size;x++){
    sumx+=hist->v[x];
    sumxx+=(hist->v[x]*hist->v[x]);
    n++;
  }
  
  *mean=sumx/n;
  *sd=(sumxx-((sumx*sumx)/n)) / n;
}













