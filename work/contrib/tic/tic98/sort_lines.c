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

#include "line.h"


static int cmpint(const void *e1, const void *e2)
{
    return ( ((int*)e1) - ((int*)e2) );
}

static int CmpOnY(const void *e1, const void *e2)
{
    return ( (((marktype*)e1)->ypos+((marktype*)e1)->h-1)  
	    - (((marktype*)e2)->ypos+((marktype*)e2)->h-1));
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





#define NEW_LINE {\
  num_lines++;\
  REALLOC(lines,num_lines,linetype);\
  line_init(&lines[num_lines-1]);\
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
    int num_lines=0;
    linetype *lines=NULL;
    
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


    start=end=0;
    while((start<profile.size) && (end<profile.size)){
      NEW_LINE;
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
	  line_addmark(&lines[num_lines-1],&step->data);
	}
      }
    }

    for(i=0;i<num_lines;i++){
      line_sort_xpos(&lines[i]);
    }

    listret=lines_to_listcopy(&lines,num_lines);

    for(i=0;i<num_lines;i++)
      line_free(&lines[i]);
    
    FREE(lines);
    
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
    int num_lines=0;
    linetype *lines=NULL;
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
	line_addmark(&lines[num_lines-1],&all_marks[i]);
      } else {
	/* start of a new line */
	NEW_LINE;
	baseline=all_marks[i].ypos+all_marks[i].h-1;
	line_addmark(&lines[num_lines-1],&all_marks[i]);
      }
    }

    for(i=0;i<num_lines;i++){
      line_sort_xpos(&lines[i]);
    }

    listret=lines_to_listcopy(&lines,num_lines);

    for(i=0;i<num_lines;i++)
      line_free(&lines[i]);
    
    FREE(lines);

    FREE(all_marks);

    return listret;
}

	    

#define TENTATIVE 25


marklistptr sortmarks_howard(const marklistptr listofmarks, 
			  const int imagew,
			  const int imageh,
			  const double skew_of_page, /* in rads */
			  const int DPI)
{
  marklistptr step,listret=NULL;
  marktype *all_marks_rot=NULL;
  int baseline;
  int i,j;
  int num_lines=0;
  int len;
  int act;
  int bots[TENTATIVE];
  linetype *lines=NULL;
  linetype line_indirect;


  rot_check();

  if(listofmarks==NULL)
    return NULL;

  if((imagew==0) || (imageh==0))
    return listofmarks;

  assert(DPI);
    

  line_init(&line_indirect);
  line_add(&line_indirect,listofmarks);

  len=marklist_length(listofmarks);


  CALLOC(all_marks_rot,len,marktype);

  /* assume the input arrives in order of extraction */
  for(i=0,step=listofmarks; step ;i++,step=step->next){
    double x,y,w,h,oldh;
    step->data.symnum=i;
    all_marks_rot[i]=step->data;

    x=step->data.xpos;
    y=step->data.ypos;
    /* it's -ve because you want to rotate it the opposite way */
    /* but it's -ve that again, because y increases downwards */
    rot(&x,&y,skew_of_page,imagew/2.0,imageh/2.0);
    all_marks_rot[i].xpos=ROUND(x);
    all_marks_rot[i].ypos=ROUND(y);

    w=step->data.w;
    h=step->data.h;
    assert(w && h);
    rot(&w,&h,skew_of_page,0,0);
    all_marks_rot[i].w=ROUND(w);

    if(all_marks_rot[i].w==0){
      all_marks_rot[i].w++;
    }

    if(all_marks_rot[i].h==0){
      all_marks_rot[i].h++;
    }

    all_marks_rot[i].start_of_line=0;
  }
  qsort((void*)all_marks_rot,(unsigned int)len,sizeof(marktype),CmpOnY);

  while(len>0){
    act=MIN(TENTATIVE,len);
    for(i=0;i<act;i++){
      bots[i]=all_marks_rot[i].ypos + all_marks_rot[i].h-1;

    }
    qsort((void*)bots,(unsigned int)act,sizeof(int),cmpint);
    /* find the median */
    baseline=bots[act/2];

    /* copy the right marks, and remove them from the list */
      
    NEW_LINE;
    for(i=0,j=0;j<len;i++,j++){
      if(i!=j){
	all_marks_rot[i]=all_marks_rot[j];
      }	  
      if((all_marks_rot[i].ypos<=baseline) ){
	line_addmark(&lines[num_lines-1],&all_marks_rot[i]);
	i--;
      } 
    }
    len=i;
    line_sort_xpos(&lines[num_lines-1]);
    line_stats_indirect(&lines[num_lines-1],&line_indirect);
  }
    
  /*listret=lines_to_listcopy(&lines,num_lines);*/
  listret=lines_to_listcopy_indirect(&lines,&line_indirect,num_lines);
    
    
  for(i=0;i<num_lines;i++)
    line_free(&lines[i]);
    
  FREE(lines);

  line_free(&line_indirect);

  FREE(all_marks_rot);


  return listret;
}

