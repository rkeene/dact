/*
 * Module: sort_lines
 *
 * This file contains functions which sort the marks into a
 * "readable sequence" Sorts the marks by their y baseline then
 * by their x centroid.
 *
 * Author:
 *   Stuart Inglis (singlis@internz.co.nz)
 *   (c) 1998
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "globals.h"

#include "marklist.h"
#include "sort_lines.h"
#include "vector.h"


static int cmpint(const void *e1, const void *e2)
{
    return ( ((int*)e1) - ((int*)e2) );
}

static int CmpOnY(const void *e1, const void *e2)
{
    return ( (((marktype*)e1)->ypos+((marktype*)e1)->h-1)  
	    - (((marktype*)e2)->ypos+((marktype*)e2)->h-1));
}
				    
static int CmpOnX(const void *e1, const void *e2)
{
    return ( (((marktype*)e1)->xpos+((marktype*)e1)->xcen)  
	    - (((marktype*)e2)->xpos+((marktype*)e2)->xcen));
}

static int CmpOnXpos(const void *e1, const void *e2)
{
    return ( (((marktype*)e1)->xpos)  
	    - (((marktype*)e2)->xpos) );
}

				    

/* a +ve rotation rotates anti-clockwise from east */
/* 
 * (100,0) rotated 90 degrees is (100,0)
 *
 *     (0,100)
 *     |
 *     |
 *     |
 *     |
 *     |
 *   --+---------- (100,0)
 *
 */

void rot(double *x, double *y, const double theta, 
	 const double cx, const double cy)
{
  double xx,yy;
  *x-=cx;
  *y-=cy;
  xx=(*x)*cos(theta) - (*y)*sin(theta);
  yy=(*x)*sin(theta) + (*y)*cos(theta);
  *x=xx+cx;
  *y=yy+cy;
}


void rot_check()
{
  double xx,yy;

  xx=100;   yy=0;
  rot(&xx,&yy,DEG2RAD(90),0,0);
  if(!(fabs(xx-0)<1e-8) && (fabs(yy-100)<1e-8)){
    fprintf(stderr,"rot() violated! %g %g\n",xx,yy);
  }

  xx=100; yy=0;
  rot(&xx,&yy,-DEG2RAD(90),0,0);
  if(!(fabs(xx-0)<1e-8) && (fabs(yy+100)<1e-8)){
    fprintf(stderr,"rot() violated! %g %g\n",xx,yy);
  }
}

	    

marklistptr sortmarks_random(const marklistptr listofmarks, 
		      const int imagew,
		      const int imageh,
		      const double rotated_angle, 
		      const int DPI)
{
    marklistptr step,listret=NULL;
    marktype *row_of_marks=NULL;
    marktype *random_rows=NULL;
    int i,len,left;
    
    if(listofmarks==NULL)
      return NULL;

    if((imagew==0) || (imageh==0))
      return listofmarks;

    assert((rotated_angle>=-180) &&(rotated_angle<=180));
    assert(DPI);

    len=marklist_length(listofmarks);

    CALLOC(row_of_marks,len,marktype);
    CALLOC(random_rows,len,marktype);

    for(i=0,step=listofmarks; step ;i++,step=step->next){
      row_of_marks[i]=marktype_copy(step->data);
    }
    
    listret=NULL;
    
    srandom(len);
    left=len;
    for(i=0;i<len;i++){
      int k;
      
      k=random()%left;
      
      random_rows[i]=row_of_marks[k];
      row_of_marks[k]=row_of_marks[left-1];
      left--;
    }

    for(i=0; i<len ;i++){
      if(listret==NULL)
	step=marklist_add(&listret,random_rows[i]);
      else
	step=marklist_add(&step,random_rows[i]);
    }

    FREE(row_of_marks);
    FREE(random_rows);


    return listret;
}

	    
typedef struct {
  marktype *marks;
  int num;
} rowtype;

void
rowtype_init(rowtype *r)
{
  r->marks=NULL;
  r->num=0;
}

void 
rowtype_free(rowtype *r)
{
  if(r->marks)
    FREE(r->marks);
  rowtype_init(r);
}

void
rowtype_add(rowtype *r, marktype *m)
{
  r->num++;
  REALLOC(r->marks,r->num,marktype);
  r->marks[r->num-1]=*m;
}

void
rowtype_print(rowtype *r)
{
  int i;

  for(i=0;i<r->num;i++){
    fprintf(stderr,"%d ",r->marks[i].ypos);
  }
  fprintf(stderr,"\n");
}


marklistptr 
rowtype_to_list(marktype *all_marks, rowtype *rows, int num_rows)
{
  int i;
  marklistptr step,listret=NULL;
  
  /* sort based on the x centroid */
  for(i=0;i<num_rows;i++){
    qsort((void*)rows[i].marks,(unsigned int)rows[i].num,
	  sizeof(marktype),CmpOnXpos);
  }
  
  /* put them back into the list in order */
  for(i=0;i<num_rows;i++){
    float xi=0,yi=0,xiyi=0,xi2=0,tx,ty,a,b;
    int n=0,j;
    
    
    for(j=0;j<rows[i].num;j++){
      n++;
      xi+=(tx=(rows[i].marks[j].xpos));
      yi+=(ty=(rows[i].marks[j].ypos+rows[i].marks[j].h-1));
      xi2+=(tx*tx);
      xiyi+=(tx*ty);
    }
    b=xiyi - (xi*yi/n);
    b=b/(xi2 - (xi*xi/n));
    
    a=yi/n - b*(xi/n);
    /*      if(n>10)
	    rows[i].marks[0].skew_of_line=-atan(b); // original at top 
	    else*/
    rows[i].marks[0].skew_of_line=0;
    
    //      fprintf(stderr,"skew of line: %d = %f\n",i,rows[i].marks[0].skew_of_line);
    
    
    for(j=0;j<rows[i].num;j++){
      marktype copy;
      
      copy=all_marks[rows[i].marks[j].symnum];
      
      if(j==0){
	copy.start_of_line=1;
	copy.ypos_of_line=ROUND(a);
      } else if(j==rows[i].num-1){
	copy.start_of_line=2;
      } else {
	copy.start_of_line=0;
      }
      
      
      if(listret==NULL){
	step=marklist_addcopy(&listret,copy);
      } else {
	step=marklist_addcopy(&step,copy);
      }
    }
  }
  return listret;
}


marklistptr 
rowtype_to_list_old(rowtype *rows, int num_rows)
{
  int i;
  marklistptr step,listret=NULL;
  
  /* sort based on the x centroid */
  for(i=0;i<num_rows;i++){
    qsort((void*)rows[i].marks,(unsigned int)rows[i].num,
	  sizeof(marktype),CmpOnXpos);
  }
  
  /* put them back into the list in order */
  for(i=0;i<num_rows;i++){
    float xi=0,yi=0,xiyi=0,xi2=0,tx,ty,a,b;
    int n=0,j;
    
    
    for(j=0;j<rows[i].num;j++){
      n++;
      xi+=(tx=(rows[i].marks[j].xpos));
      yi+=(ty=(rows[i].marks[j].ypos+rows[i].marks[j].h-1));
      xi2+=(tx*tx);
      xiyi+=(tx*ty);
    }
    b=xiyi - (xi*yi/n);
    b=b/(xi2 - (xi*xi/n));
    
    a=yi/n - b*(xi/n);
    /*      if(n>10)
	    rows[i].marks[0].skew_of_line=-atan(b); // original at top 
	    else*/
    rows[i].marks[0].skew_of_line=0;
    
    //      fprintf(stderr,"skew of line: %d = %f\n",i,rows[i].marks[0].skew_of_line);
    
    
    for(j=0;j<rows[i].num;j++){
      if(j==0){
	rows[i].marks[j].start_of_line=1;
	rows[i].marks[j].ypos_of_line=ROUND(a);
      }
      else
	rows[i].marks[j].start_of_line=0;
      
      if(listret==NULL)
	step=marklist_addcopy(&listret,rows[i].marks[j]);
      else
	step=marklist_addcopy(&step,rows[i].marks[j]);
    }
  }
  return listret;
}




#define NEW_ROW {\
  num_rows++;\
  REALLOC(rows,num_rows,rowtype);\
  rowtype_init(&rows[num_rows-1]);\
}




marklistptr sortmarks_profile(const marklistptr listofmarks, 
		      const int imagew,
		      const int imageh,
		      const double rotated_angle, 
		      const int DPI)
{
    marklistptr step,listret=NULL;
    int i,pixels;
    int start,end,len;
    vectortype profile;
    double x,y;
    int num_rows=0;
    rowtype *rows=NULL;
    
    if(listofmarks==NULL)
      return NULL;

    if((imagew==0) || (imageh==0))
      return listofmarks;

    assert((rotated_angle>=-180) &&(rotated_angle<=180));
    assert(DPI);

    len=marklist_length(listofmarks);

    pixels=mm2pixels(globals.g_reading_parameter,DPI);
    assert(pixels<=10000);

    vector_alloc(&profile, imageh*3);
    for(step=listofmarks;step;step=step->next){
      x=step->data.xpos;
      y=step->data.ypos+step->data.h-1;
      rot(&x,&y,DEG2RAD(-5),imagew/2.0,imageh/2.0);
      
      assert(imageh+(int)y >=0);
      assert(imageh+(int)y < profile.size);
      profile.v[imageh+(int)y]++;
    }
    for(i=0;i<profile.size;i++){
      profile.v[i]=(profile.v[i]*profile.v[i]);
    }

    if(pixels){
      vector_convolve(&profile,pixels,RECTANGULAR);
    }
    
    vector_norm(&profile);
    vector_threshold(&profile,0.3,0,1);
    vector_invert(&profile);
    vector_mean(&profile);

    {
      int rows=1;
      for(i=0;i<profile.size;i++){
	if(profile.v[i]>0.99)
	  rows++;
      }
      fprintf(stderr,"rows: %d\n",rows);
    }

    start=end=0;
    while((start<profile.size) && (end<profile.size)){
      NEW_ROW;
      start=end;
      end++;
      while((end<profile.size) && (profile.v[end]<0.99)){
	end++;
      }

      for(step=listofmarks;step;step=step->next){
	double x,y;

	x=step->data.xpos;
	y=step->data.ypos+step->data.h-1;
	rot(&x,&y,DEG2RAD(-5),imagew/2.0,imageh/2.0);

	if((y>=start) && (y<end)){
	  rowtype_add(&rows[num_rows-1],&step->data);
	}
      }
    }
    fprintf(stderr,"rows: %d\n",num_rows);

    listret=rowtype_to_list_old(rows,num_rows);

    for(i=0;i<num_rows;i++){
      rowtype_free(&rows[i]);
    }
    FREE(rows);

    vector_free(&profile);

    return listret;
}








marklistptr sortmarks_zhang(const marklistptr listofmarks, 
		      const int imagew,
		      const int imageh,
		      const double rotated_angle, 
		      const int DPI)
{
    marklistptr step,listret=NULL;
    marktype *all_marks=NULL;
    int num_rows=0;
    rowtype *rows=NULL;
    int baseline;
    int i,len;
    
    if(listofmarks==NULL)
      return NULL;

    if((imagew==0) || (imageh==0))
      return listofmarks;

    assert((rotated_angle>=-180) &&(rotated_angle<=180));
    assert(DPI);

    len=marklist_length(listofmarks);

    CALLOC(all_marks,len,marktype);

    for(i=0,step=listofmarks; step ;i++,step=step->next){
      all_marks[i]=step->data;
    }

    qsort((void*)all_marks,(unsigned int)len,sizeof(marktype),CmpOnY);

    baseline=-1;
    for(i=0;i<len;i++){
      int t;

      t=all_marks[i].ypos;
      assert(t>=0);

      /* if t is higher than the baseline */
      if(t<=baseline){
	/* on line */
	rowtype_add(&rows[num_rows-1],&all_marks[i]);
      } else {
	/* start of a new line */
	NEW_ROW;
	baseline=all_marks[i].ypos+all_marks[i].h-1;
	rowtype_add(&rows[num_rows-1],&all_marks[i]);
      }
    };

    listret=rowtype_to_list(all_marks,rows,num_rows);

    for(i=0;i<num_rows;i++){
      rowtype_free(&rows[i]);
    }

    FREE(rows);
    for(i=0;i<len;i++)
      marktype_free(&all_marks[i]);
    FREE(all_marks);

    return listret;
}

	    

#define TENTATIVE 25

marklistptr sortmarks_howard(const marklistptr listofmarks, 
			  const int imagew,
			  const int imageh,
			  const double rotated_angle, 
			  const int DPI)
{
    marklistptr step,listret=NULL;
    marktype *all_marks=NULL;
    marktype *all_marks_rot=NULL;
    int baseline;
    int i,j;
    int num_rows=0;
    rowtype *rows=NULL;
    int len;
    int act;
    int bots[TENTATIVE];
    marktype im;

    rot_check();

    if(listofmarks==NULL)
      return NULL;

    if((imagew==0) || (imageh==0))
      return listofmarks;

    assert((rotated_angle>=-180) &&(rotated_angle<=180));
    assert(DPI);
    
    len=marklist_length(listofmarks);

    marklist_reconstruct(listofmarks,&im);


    CALLOC(all_marks,len,marktype);
    CALLOC(all_marks_rot,len,marktype);

    /* assume the input arrives in order of extraction */
    for(i=0,step=listofmarks; step ;i++,step=step->next){
      double x,y,w,h;
      step->data.symnum=i;
      all_marks[i]=step->data;
      all_marks_rot[i]=step->data;
      x=step->data.xpos;
      y=step->data.ypos;
      /* it's -ve because you want to rotate it the opposite way */
      /* but it's -ve that again, because y increases downwards */
      rot(&x,&y,globals.g_skew,imagew/2.0,imageh/2.0);
      all_marks_rot[i].xpos=ROUND(x);
      all_marks_rot[i].ypos=ROUND(y);

      w=step->data.w;
      h=step->data.h;
      rot(&w,&h,globals.g_skew,0,0);
      all_marks_rot[i].w=ROUND(w);
      all_marks_rot[i].w=ROUND(h);
    }
    qsort((void*)all_marks_rot,(unsigned int)len,sizeof(marktype),CmpOnY);

    while(len>0){
      act=MIN(TENTATIVE,len);
      for(i=0;i<act;i++){
	bots[i]=all_marks_rot[i].ypos+all_marks_rot[i].h-1;
      }
      qsort((void*)bots,(unsigned int)act,sizeof(int),cmpint);
      /* find the median */
      baseline=bots[act/2];
      
      /*      fprintf(stderr,"row %d: %d (%d)\n",num_rows, baseline,len);*/
      
      NEW_ROW;
      for(i=0;i<len;i++){
	if(all_marks_rot[i].ypos<=baseline){
	  rowtype_add(&rows[num_rows-1],&all_marks_rot[i]);
	} 
      }
      /* remove them */

      /*

      for(i=0;i<len;i++){
	int k,p;
	if(all_marks[i].ypos<=baseline){
	  k=i;p=0;
	  while((k<len) && (all_marks[k].ypos<=baseline)){
	    k++;
	    p++;
	  }
	  for(j=k+1;j<len;j++)
	    all_marks[j-k+i]=all_marks[j];
	  len--;
	  i--;
	}
      }
      */

      for(i=0;i<len;i++){
	if(all_marks_rot[i].ypos<=baseline){
	  for(j=i+1;j<len;j++)
	    all_marks_rot[j-1]=all_marks_rot[j];
	  len--;
	  i--;
	}
      }

    }

    /*    fprintf(stderr,"num_rows %d\n",num_rows);*/

    listret=rowtype_to_list(all_marks,rows,num_rows);


    for(step=listret; step ;step=step->next){
      if(step->data.start_of_line){
	if(step->data.start_of_line==1){
	  for(i=0;i<=20;i++){
	    ppix(im,step->data.xpos+i,step->data.ypos+step->data.h,1);
	    ppix(im,step->data.xpos+i,step->data.ypos+step->data.h+1,1);
	  }
	  for(i=-20;i<=10;i++){
	    ppix(im,step->data.xpos,step->data.ypos+step->data.h+i,1);
	    ppix(im,step->data.xpos+1,step->data.ypos+step->data.h+i,1);
	  }
	}

	else if(step->data.start_of_line==2){
	  for(i=-20;i<=0;i++){
	    ppix(im,step->data.xpos+step->data.w+i,step->data.ypos+step->data.h,1);
	    ppix(im,step->data.xpos+step->data.w+i,step->data.ypos+step->data.h+1,1);
	  }
	  for(i=-20;i<=10;i++){
	    ppix(im,step->data.xpos+step->data.w,step->data.ypos+step->data.h+i,1);
	    ppix(im,step->data.xpos+step->data.w+1,step->data.ypos+step->data.h+i,1);
	  }
	}
      }
    }
	

    marktype_writenamed("a.pbm",im);
    marktype_free(&im);


    for(i=0;i<num_rows;i++){
      rowtype_free(&rows[i]);
    }
    FREE(rows);
    FREE(all_marks);
    FREE(all_marks_rot);


    return listret;
}

