/*
 * Module: docstrum
 *
 * Implements a k-nearest neighbour method for calculating
 * skew. Usually we use k=5, and perform a polar integration
 * apply a wide smoothing filter.
 *
 *
 * Author:
 *   Stuart Inglis (singlis@internz.co.nz)
 *   (c) 1998 
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include "boundary.h"

#include "marklist.h"
#include "windowing.h"

#include "docstrum.h"
#include "sort_lines.h"

#include "globals.h"


typedef struct {
    int marknum;
    double x,y;
} pairtype;

#ifndef DBL_MAX
#define DBL_MAX 9e9
#endif


/* 
   returns between -pi and +pi, ie. [-180 to +180)

                  | +pi/2 (90)
                  |
                  |
                  |
-180   -----------+-------------  0 
(-pi)             |
                  |
                  |
                  | -pi/2 (-90)

to normalise between [-90, 90),
  if(ang>90) ang=ang-180;
  else if (ang<-90) ang=ang+180;
*/
static double 
atan_xy(double yd, double xd)
{
    double angle;
    
    angle=atan2(yd,xd);  /* returns a result between (-PI, PI] */

    while(angle>=PI) 
	angle=angle-2*PI;/* now returns a result between [-PI, PI) */
 
/*    assert((angle>=-PI) && (angle<PI));*/

    return angle;
}




static int 
compare(const void *e1, const void *e2)
{
    if ( ((pairtype*)e1)->x < ((pairtype*)e2)->x )
	return -1;
    else if ( ((pairtype*)e1)->x > ((pairtype*)e2)->x )
	return 1;
    else return 0;
}



static void 
neighbours_free(neighbourtype *ary,  int number_marks)
{
  if(ary){
    int i;
    for(i=0;i<number_marks;i++)
      FREE(ary[i].neighs);
    FREE(ary);
  }
}



/*
**
** 
**
**
**
*/
static neighbourtype * 
calc_k_neighbours(marklistptr list, int k)
{
    marklistptr step;
    int number_marks,marknum,i;
    double *NNx,*NNy,*NNdist;
    int *NNnum;
    double xd,yd;
    int idx;
    pairtype *biglist;
    neighbourtype *ary;
    marktype temp;
    marklistptr *fastaccess;

    number_marks=marklist_length(list);

    if(number_marks==0){
      return NULL;
    }

    CALLOC(fastaccess,number_marks,marklistptr);
    for(idx=0,step=list;step;step=step->next,idx++){
      fastaccess[idx]=step;
    }


    /* allocate the array that we will return */
    CALLOC(ary,number_marks,neighbourtype);
    for(i=0;i<number_marks;i++)
	CALLOC(ary[i].neighs,k,neighbourlisttype);

    /* then alloc temp arrays */
    CALLOC(NNx,k,double);
    CALLOC(NNy,k,double);
    CALLOC(NNdist,k,double);
    CALLOC(NNnum,k,int);
    CALLOC(biglist,number_marks,pairtype);
    for(step=list,idx=0; step; step=step->next,idx++){
	biglist[idx].marknum=idx;
	biglist[idx].x=step->data.xpos+step->data.d_xcen;
	biglist[idx].y=step->data.ypos+step->data.d_ycen;
    }
    
    qsort((void*)biglist,(unsigned int)number_marks,sizeof(pairtype),compare);
    

    for(marknum=0;marknum<number_marks;marknum++)  {
	int up,down,xpos,ypos,j;
	double dist;

	/*if((marknum%100)==0)fprintf(stderr,"%d\r",marknum);*/
	xpos=(int)biglist[marknum].x;
	ypos=(int)biglist[marknum].y;
	
	/* NNdist[0] is the closest, NNdist[k-1] is far away */
	for(i=0;i<k;i++){
	  NNdist[i]=NNx[i]=NNy[i]=DBL_MAX;
	    NNnum[i]=-1;
	}
	down=marknum-1;
	up=marknum+1;
	while((down>=0) || (up<number_marks)){
	    if(down>=0){
		xd=(biglist[down].x-xpos);
		yd=(biglist[down].y-ypos);
		
		dist=sqrt(xd*xd+yd*yd);
		if(fabs(xd)>NNdist[k-1]) down=0;
		else if(dist<NNdist[k-1]) {
		    i=0;
		    while((dist>NNdist[i])&&(i<(k-1))) i++;
		    /* add at i'th position */
		    for(j=k-2;j>=i;j--){
			NNdist[j+1]=NNdist[j];
			NNx[j+1]=NNx[j];
			NNy[j+1]=NNy[j];
			NNnum[j+1]=NNnum[j];
		    }
		    NNdist[i]=(dist); /* actually store here */
		    NNx[i]=xd;
		    NNy[i]=yd;
		    NNnum[i]=down;
		}
		down--;
	    }
	    if(up<number_marks){
		xd=(biglist[up].x-xpos);
		yd=(biglist[up].y-ypos);
		
		dist=sqrt(xd*xd+yd*yd);
		if(fabs(xd)>NNdist[k-1]) up=number_marks;
		else if(dist<NNdist[k-1]) {
		    i=0;
		    while((dist>NNdist[i])&&(i<(k-1))) i++;
		    /* add at i'th position */
		    for(j=k-2;j>=i;j--){
			NNdist[j+1]=NNdist[j];
			NNx[j+1]=NNx[j];
			NNy[j+1]=NNy[j];
			NNnum[j+1]=NNnum[j];
		    }
		    NNdist[i]=(dist); /* actually store here */
		    NNx[i]=xd;
		    NNy[i]=yd;
		    NNnum[i]=up;
		}
		up++;

	    }
        }
	ary[marknum].num=biglist[marknum].marknum;
	temp=fastaccess[ary[marknum].num]->data;
	ary[marknum].x=temp.xpos+temp.d_xcen;
	ary[marknum].y=temp.ypos+temp.d_ycen;
	ary[marknum].num_neighs=0;
	for(i=0;i<k;i++) if (NNnum[i]>=0) {
	    ary[marknum].num_neighs++;
	    ary[marknum].neighs[i].xd=NNx[i];
	    ary[marknum].neighs[i].yd=NNy[i];
	    ary[marknum].neighs[i].num=NNnum[i];
	}
    }
    FREE(NNnum);
    FREE(NNx);
    FREE(NNy);
    FREE(NNdist);
    FREE(biglist);
    FREE(fastaccess);

    /* finished with all the marks */
    return ary;
}

/*
 * calc_docstrum_using_neighs()
 *
 * Returns 0 if successful, otherwise 1.
 *
 */
static int
calc_docstrum_using_neighs(marklistptr list, 
			   neighbourtype *neighs, 
			   double *skew)
{
    double *hist=NULL;
    int number_marks,i,j;
    double xd,yd;
    double ang;
    int idx,ret;
    double maxp,dist;
    double maxval;
    int marknum;
    double SCALE=0.1;
    double lowrange=-90;
    double highrange=90;
    double SM_SIZE=10; /* size of the smoothing filter in degrees */
    FILE *fp=NULL;

    /*fp=fopen("docstrum.data","wb");
    if(!fp) {
      fprintf(stderr,"Failing to open: 'docstrum'\n");fp=NULL;
    }*/

    if(neighs==NULL){
      *skew=0;
      return 1;
    }

    CALLOC(hist,ROUND(180/SCALE)+1,double);

    number_marks=marklist_length(list);
    
    /* found the k nearest neighbours */
    
    for(marknum=0;marknum<number_marks;marknum++){
	for(i=0;i<neighs[marknum].num_neighs;i++){
	    xd=neighs[marknum].neighs[i].xd;
	    yd=neighs[marknum].neighs[i].yd;
	    dist=sqrt(xd*xd+yd*yd);
	    
	    if(fp)fprintf(fp,"%g %g\n",xd,-yd);
	    
	    ang=atan_xy(-yd,xd);   /* -yd as the original is at the top */

	    ang=RAD2DEG(ang);

	    if(ang>=90) ang=ang-180;
	    else if (ang<-90) ang=ang+180;  /* normalise between [-90,90) */

	    {
		idx=ROUND((ang/SCALE)  +90/SCALE);
		idx=idx%(ROUND(180/SCALE));
		assert(idx>=0);
		assert(idx<(ROUND(180/SCALE)));

		hist[idx]+=1.0;
	    }

	}
    }
    if(fp)fclose(fp);
    
    /* smooth the NN angle histogram */
    
    if(SM_SIZE>0) {
	double *newhist=NULL;
	double *filt=NULL;
	int filtsize;
	
	/*	write_window_labelled("hist.orig",hist,0,ROUND(180/SCALE)-1,-90.0,90.0,SCALE);*/

	CALLOC(newhist,ROUND(180/SCALE)+1,double);
	
	filtsize=ROUND(SM_SIZE/SCALE);
	if((filtsize%2)==0) filtsize++; /* so it's odd */
	
	CALLOC(filt,filtsize,double);
	create_window(filt,filtsize,HAMMING);
	convolve(hist,0,ROUND(180/SCALE)-1, filt, filtsize, filtsize/2, newhist, CYCLIC);
	FREE(filt);
	
	for(i=0;i<ROUND(180/SCALE);i++)
	    hist[i]=newhist[i]/filtsize;
	FREE(newhist);
	
	/*write_window_labelled("histogram.data",hist,0,ROUND(180/SCALE)-1,-90.0,90.0,SCALE);*/

    }
    
    maxp=maxval=-1;
    for(i=ROUND(lowrange/SCALE);i<ROUND(highrange/SCALE);i++){
	j=i+ROUND(90/SCALE);
	if(hist[j]>maxval) {maxval=hist[j]; maxp=i;}
    }
    /* found maxval, now average all points with this value */
    maxp=0;j=0;
    for(i=ROUND(lowrange/SCALE);i<ROUND(highrange/SCALE);i++){
	if(hist[i+ROUND(90/SCALE)]==maxval) {maxp+=i;j++;}
    }
    maxp/=j;
    
    maxp=maxp*SCALE;
    
    FREE(hist);

    /* was the peak high enough?, values around 1 are for
     * very small bitmaps. */
    if(maxval<0.7){
      *skew=0;
      ret=1;
    } else {
      *skew=maxp;
      ret=0;
    }

    return ret;
}










double
calc_docstrum_spacing(marklistptr list, 
			     neighbourtype *neighs,
			     double orientation,
			     double *spacing_within,
			     double *spacing_between
			     )
{
    double *hist_between=NULL;
    double *hist_within=NULL;
    int marknum,number_marks;
    double xd,yd;
    double ang;
    int i;
    double orientation_perp;
    double dist;


    if(0)fprintf(stderr,"calc_docstrum_spacing\n");

    orientation_perp=orientation;
    if(orientation_perp>=0) orientation_perp-=90;  
    else orientation_perp+=90;


    CALLOC(hist_between,200,double);
    CALLOC(hist_within,200,double);

    number_marks=marklist_length(list);

    /* found the k nearest neighbours */
    
    for(marknum=0;marknum<number_marks;marknum++){
	for(i=0;i<neighs[marknum].num_neighs;i++){
	    xd=neighs[marknum].neighs[i].xd;
	    yd=neighs[marknum].neighs[i].yd;
	    dist=sqrt(xd*xd+yd*yd);

	    ang=atan_xy(-yd,xd); /* bitmaps have 0 at top */

	    ang=RAD2DEG(ang);

	    if(ang>=90) ang-=180;
	    else if (ang<-90) ang+=180;	/* normalise between [-90,90) */
	    
	    if( (fabs(ang-orientation)<=30) ) {
		if(ROUND(dist)<200)
		    hist_within[ROUND(dist)]+=1.0;
	    }
	    
	    /* perpendicular */
	    if(fabs(ang-orientation_perp)<=30) {
		if(ROUND(dist)<200)
		    hist_between[ROUND(dist)]+=1.0;
	    }

	}
    }

    /* smooth the hist_between */
    { double *newhist=NULL;
      double *filt;
      int filtsize;
      
      if(0)fprintf(stderr,"windowing histogram_between\n");

      CALLOC(newhist,200,double);

      filtsize=11;
      CALLOC(filt,filtsize,double);
      create_window(filt,filtsize,RECTANGULAR);
      convolve(hist_between,0,199, filt, filtsize, filtsize/2, newhist, 0);
      FREE(filt);
	      
      for(i=0;i<200;i++)
	  hist_between[i]=newhist[i];
      FREE(newhist);

  }

    /* smooth the hist_within */
    { double *newhist=NULL;
      double *filt;
      int filtsize;
      
      if(0)fprintf(stderr,"windowing histogram_within\n");

      CALLOC(newhist,200,double);

      filtsize=11;
      CALLOC(filt,filtsize,double);
      create_window(filt,filtsize,RECTANGULAR);
      convolve(hist_within,0,199, filt, filtsize, filtsize/2, newhist, 0);
      FREE(filt);
	      
      for(i=0;i<200;i++)
	  hist_within[i]=newhist[i];
/*      write_window_labelled("hist_within",hist_within,0,199,0,199,1);*/
      FREE(newhist);
  }

    
    { double maxval,maxp;
      int j;
      
      maxp=0;
      maxval=hist_within[0];
      for(i=0;i<200;i++)
	  if(hist_within[i]>maxval) {maxval=hist_within[i]; maxp=i;}
      /* found maxval, now average all points with this value */
      maxp=0;j=0;
      for(i=0;i<200;i++)
	  if(hist_within[i]==maxval) {maxp+=i;j++;}
      maxp/=j;
      
      *spacing_within=maxp;

      maxp=0;
      maxval=hist_between[0];
      for(i=0;i<200;i++)
	  if(hist_between[i]>maxval) {maxval=hist_between[i]; maxp=i;}
      /* found maxval, now average all points with this value */
      maxp=0;j=0;
      for(i=0;i<200;i++)
	  if(hist_between[i]==maxval) {maxp+=i;j++;}
      maxp/=j;
      
      *spacing_between=maxp;

      }
    FREE(hist_between);
    FREE(hist_within);

    return 0;
}









void
prune_neighbours_angle_distance(neighbourtype *neighs,
		      int number_marks,
		      double orientation,
		      double orient_threshold,
		      double lesser)
{
    int i;
    double xd,yd,dist,ang;
    int marknum;

    
    for(marknum=0;marknum<number_marks;marknum++)
	for(i=0;i<neighs[marknum].num_neighs;i++){
	    xd=neighs[marknum].neighs[i].xd;
	    yd=neighs[marknum].neighs[i].yd;
	    dist=sqrt(xd*xd+yd*yd);

	    ang=atan_xy(-yd,xd); /* bitmaps have 0 at top */

	    ang=RAD2DEG(ang);

	    if(ang>=90) ang-=180;
	    else if (ang<-90) ang+=180;	/* normalise between [-90,90) */
	    
	    if(!( (fabs(ang-orientation)<=orient_threshold) && (dist<=lesser))) {
		neighs[marknum].neighs[i].xd=
		neighs[marknum].neighs[i].yd=0.0;
		neighs[marknum].neighs[i].num=-1;
	    }
	}

    /* pack the links we've removed */
    for(marknum=0;marknum<number_marks;marknum++){
	int cc,num_n;
	cc=0;
	for(i=0;i<neighs[marknum].num_neighs;i++)
	    if(neighs[marknum].neighs[i].num>=0) 
		cc++;
	if(cc!=neighs[marknum].num_neighs){
	    num_n=neighs[marknum].num_neighs;
	    neighs[marknum].num_neighs=0;
	    for(i=0;i<num_n;i++)
		if(neighs[marknum].neighs[i].num>=0){
		    neighs[marknum].neighs[ neighs[marknum].num_neighs]=neighs[marknum].neighs[i];
		    neighs[marknum].num_neighs++;
		}
	}
    }
}
		    

void 
prune_neighbours_distance(neighbourtype *neighs,
		      int number_marks,
		      double distance)
{
    int i;
    double xd,yd,dist,ang;
    int marknum;

    
    for(marknum=0;marknum<number_marks;marknum++)
	for(i=0;i<neighs[marknum].num_neighs;i++){
	    xd=neighs[marknum].neighs[i].xd;
	    yd=neighs[marknum].neighs[i].yd;
	    dist=sqrt(xd*xd+yd*yd);

	    ang=atan_xy(-yd,xd); /* bitmaps have 0 at top */

	    ang=RAD2DEG(ang);

	    if(ang>=90) ang-=180;
	    else if (ang<-90) ang+=180;	/* normalise between [-90,90) */
	    
	    if(dist>distance){
		neighs[marknum].neighs[i].xd=
		neighs[marknum].neighs[i].yd=0.0;
		neighs[marknum].neighs[i].num=-1;
	    }
	}

    /* pack the links we've removed */
    for(marknum=0;marknum<number_marks;marknum++){
	int cc,num_n;
	cc=0;
	for(i=0;i<neighs[marknum].num_neighs;i++)
	    if(neighs[marknum].neighs[i].num>=0) 
		cc++;
	if(cc!=neighs[marknum].num_neighs){
	    num_n=neighs[marknum].num_neighs;
	    neighs[marknum].num_neighs=0;
	    for(i=0;i<num_n;i++)
		if(neighs[marknum].neighs[i].num>=0){
		    neighs[marknum].neighs[ neighs[marknum].num_neighs]=neighs[marknum].neighs[i];
		    neighs[marknum].num_neighs++;
		}
	}
    }
}
		    
	













static void 
view_connections(marktype image,
		      neighbourtype *neighs,
		      int num)
{
    int i,j;

    assert(neighs);
    
    for(i=0;i<num;i++){
	for(j=0;j<neighs[i].num_neighs;j++){
	    marktype_line(image,
			  ROUND(neighs[i].x),
			  ROUND(neighs[i].y),
			  ROUND(neighs[i].x+ neighs[i].neighs[j].xd),
			  ROUND(neighs[i].y+ neighs[i].neighs[j].yd));
	}
    }
}

int 
calc_docstrum_list(marklistptr list, double *skew, int resolution)
{
  neighbourtype * neighs;
  int ret;
  int len;

  /* prune under 0.5 mm^2 */
  marklist_prune_under_area (&list,DEFAULT_MIN_AREA,resolution);

  len=marklist_length(list);

  neighs=calc_k_neighbours(list,DEFAULT_K);
  {
    marktype im;
    double an,within,between;
    int numgroups;

    calc_docstrum_spacing(list,neighs,0,&within,&between);

    prune_neighbours_distance(neighs,len, 3*between);
#ifdef 0
    prune_neighbours_angle_distance(neighs,len, 0,30, 6*within/*MIN(6*within,sqrt(2.0)*between)*/);
#endif

    /*
    marklist_reconstruct(list,&im);
    view_connections(im,neighs,len);
    marktype_writenamed("b.pbm",im);
    fprintf(stderr,"written\n");
    marktype_free(&im);
    */

    cluster_into_groups(neighs,len,&numgroups);

    /*
    marklist_reconstruct(list,&im);
    view_connections_groups(im,neighs,len,numgroups);
    marktype_writenamed("c.pbm",im);
    fprintf(stderr,"written\n");
    marktype_free(&im);
    */

  }

  ret=calc_docstrum_using_neighs(list,neighs,skew);
  neighbours_free(neighs,len);

  return ret;
}




marklistptr
docstrum_order(marklistptr list)
{
  neighbourtype * neighs;
  int len;
  marktype im;
  double an,within,between;
  int numgroups;
  marklistptr ret=NULL;
  double skew;
  
  len=marklist_length(list);

  neighs=calc_k_neighbours(list,DEFAULT_K);

  calc_docstrum_using_neighs(list,neighs,&skew);
  /*fprintf(stderr,"skew: %.2f\n",skew);*/

  skew=RAD2DEG(globals.g_skew);
  calc_docstrum_spacing(list,neighs,skew,&within,&between);
  /*  prune_neighbours_angle_distance(neighs,len, 0,45, MIN(3*within,sqrt(2.0)*between));*/
  /* this was ~around 3.0*within for docstrum experiment... damn I forgot..*/

  /*  if((globals.g_dump_zones==0)){*/
    prune_neighbours_distance(neighs,len, 5*within);
    /*  }*/

  /*
  marklist_reconstruct(list,&im);
  view_connections(im,neighs,len);
  marktype_writenamed("b.pbm",im);
  fprintf(stderr,"written\n");
  marktype_free(&im);
  */
  
  cluster_into_groups(neighs,len,&numgroups);

  /*

  marklist_reconstruct(list,&im);
  view_connections_groups(im,neighs,len,numgroups);
  marktype_writenamed("c.pbm",im);
  fprintf(stderr,"written\n");
  marktype_free(&im);
  */

  
  ret=groups_to_marklist(list,neighs,len,numgroups, skew, within);

  neighbours_free(neighs,len);

  return ret;
}






int 
calc_docstrum(marktype *image, double *skew)
{
  marklistptr list=NULL;
  marktype copy;
  int ret;

  copy=marktype_copy(*image);
  list=extract_all_marks(list,copy,1,8);
  marktype_free(&copy);

  if(image->resolution==0)
    image->resolution=300; /* defaults to 300 dpi */
  
  ret=calc_docstrum_list(list,skew,image->resolution);

  marklist_free(&list);

  return ret;
}











    
		  


void 
cluster_into_groups(neighbourtype *neighs,
		    const int number_marks,
		    int *numgroups)
{
    int marknum;
    
    /* set all the group numbers to -1 */
    for(marknum=0;marknum<number_marks;marknum++)
	neighs[marknum].group=-1;
    
    /* transitively label all the groups, starting from 0 */
    *numgroups=0;
    for(marknum=0;marknum<number_marks;marknum++){
      /*	if(neighs[marknum].group<0)*/{
	if(label_group(neighs,marknum,(*numgroups),number_marks))
	  (*numgroups)++;
	}
    }
    /* pack the groups */
    pack_groups(neighs,numgroups,number_marks);

    for(marknum=0;marknum<number_marks;marknum++)
      assert(neighs[marknum].group>=0);


}



int 
label_group(neighbourtype *neighs,
	    const int marknum,
	    const int currentgroup,
	    const int total)
{
    int i,j;
    int linknum,linkgroup;
    int ret=0;

    if(neighs[marknum].num_neighs<=0) {
      neighs[marknum].group=currentgroup;
      ret=1;
      return ret;
    }

    if(neighs[marknum].group<0){
	neighs[marknum].group=(currentgroup);
	ret=1;

	for(i=0;i<neighs[marknum].num_neighs;i++){
	    linknum=neighs[marknum].neighs[i].num;
	    linkgroup=neighs[linknum].group;
	    if(linkgroup==currentgroup)
		continue; /* don't need to worry */

	    if(linkgroup>=0){
		/* if link was already set, but they are different */
		/* setting all previous values of 'linkgroup' to 'currentgroup' */
		for(j=0;j<total;j++)
		    if(neighs[j].group==linkgroup)
			neighs[j].group=currentgroup;
	    }
	    else 
		label_group(neighs,linknum, currentgroup, total);
	}
    }
    return ret;
}

void 
pack_groups(neighbourtype *neighs, 
	    int *num_groups, 
	    const int total
	    )
{
    int *counts;
    int new_num=0,new_count=0;
    int i,k;

    if((num_groups==NULL) || (*num_groups<1)){
      return;
    }
    
    CALLOC(counts,(*num_groups),int);
    for(i=0;i<*num_groups;i++){
	counts[i]=dump_group(neighs,i,total);
	if(counts[i]!=0) new_num++;
    }
    for(i=0;i<*num_groups;i++) if((counts[i]!=0)){
	if(i!=new_count)
	    for(k=0;k<total;k++)
		if(neighs[k].group==i)
		    neighs[k].group=new_count;
	new_count++;
    }
    FREE(counts);
    *num_groups=new_num;
}    


int 
dump_group(const neighbourtype *neighs,
	   const int currentgroup,
	   const int total)
{
    int i,count=0;

    for(i=0;i<total;i++)
	if(neighs[i].group==currentgroup) count++;
    return count;
}
	

	
void 
group_reset_classifications(marklistptr list)
{
    marklistptr step;

    for(step=list; step; step=step->next)
	step->data.grp=-1;
}

/* does NOT allocate any memory */
void
group_bounds(const neighbourtype *neighs,
	     const marklistptr *fastarray,
	     const int number_marks,
	     const int group_number,
	     int *x1,int *y1, int *x2, int *y2
	     )
{
    int i,count;
    marktype *temp;
    
    *x1=*y1=INT_MAX;
    *x2=*y2=-INT_MAX;
    count=0;
    for(i=0;i<number_marks;i++) 
	if(neighs[i].group==group_number){
	    count++;
	    temp=&(fastarray[neighs[i].num]->data);
	    if(temp->xpos<*x1) *x1=temp->xpos;
	    if(temp->ypos<*y1) *y1=temp->ypos;
	    if(temp->xpos+temp->w-1 > *x2) *x2=temp->xpos+temp->w-1;
	    if(temp->ypos+temp->h-1 > *y2) *y2=temp->ypos+temp->h-1;
	}
    if(count==0)
	fprintf(stderr,"WARNING! -- group %d has no members!\n",group_number);
}


void
group_bounds_centroids(const neighbourtype *neighs,
	     const marklistptr *fastarray,
	     const int number_marks,
	     const int group_number,
	     int *x1,int *y1, int *x2, int *y2
	     )
{
    int i,count;
    marktype *temp;
    int xc,yc,_x1,_y1,_x2,_y2;
		     
    *x1=*y1=INT_MAX;
    *x2=*y2=-INT_MAX;
    count=0;
    for(i=0;i<number_marks;i++) 
	if(neighs[i].group==group_number){
	    count++;
	    temp=&(fastarray[neighs[i].num]->data);
	    xc=temp->xpos+temp->xcen;
	    yc=temp->ypos+temp->ycen;
	    _x1=temp->xpos;
	    _y1=temp->ypos;
	    _x2=temp->xpos+temp->w-1;
	    _y2=temp->ypos+temp->h-1;


	    if(_x1< *x1) *x1=_x1;
	    if(_y1< *y1) *y1=_y1;
	    if(_x2> *x2) *x2=_x2;
	    if(_y2> *y2) *y2=_y2;

	    if(*x1<0) *x1=0;
	    if(*y1<0) *y1=0;
	    if(*x2>=temp->imagew) *x2=temp->imagew-1;
	    if(*y2>=temp->imageh) *y2=temp->imageh-1;
	}
    if(count==0)
	fprintf(stderr,"WARNING! -- group %d has no members!\n",group_number);
}


void 
view_connections_groups(marktype image,
			neighbourtype *neighs,
			int number_marks,
			int number_groups)
{
  int i,j,g;

  if(0)fprintf(stderr,"overlaying connections on bitmap...\n");
  for(g=0;g<number_groups;g++) 
    for(i=0;i<number_marks;i++) if(neighs[i].group==g){
      /*fprintf(stderr,"group %d, mark %d\n",g,neighs[i].num);*/
      for(j=0;j<neighs[i].num_neighs;j++){
	/*fprintf(stderr,"->%d\n",neighs[i].neighs[j].num);*/
	marktype_line(image,
		      ROUND(neighs[i].x),
		      ROUND(neighs[i].y),
		      ROUND(neighs[i].x+ neighs[i].neighs[j].xd),
		      ROUND(neighs[i].y+ neighs[i].neighs[j].yd));
      }
    }
}


typedef struct {
  int x1,y1,x2,y2,xcen,ycen;
  int group;
} coord2d;


static int 
compareycoords(const void *e1, const void *e2)
{
    if ( ((coord2d*)e1)->ycen < ((coord2d*)e2)->ycen )
	return -1;
    else if ( ((coord2d*)e1)->ycen > ((coord2d*)e2)->ycen )
	return 1;
    else return 0;
}



void
dump_groups_with_class(neighbourtype *neighs,
		       marklistptr *array,
		       int len,
		       int number_groups)
{
  int g;
  int num_tags=0;
  int _x1[1000],_y1[1000],_x2[1000],_y2[1000],_tags[1000];
  char s[100],tag[100];
  FILE *fp=NULL;
  int thisarea=0;

  if(globals.g_layout_filename){
    fp=fopen(globals.g_layout_filename,"rt");
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
    }
  }
  

  for(g=0;g<number_groups;g++){
    int ww,thislen;
    marktype *temp;
    int x1,y1,x2,y2,tt;
    int istext, isnot;
    group_bounds_centroids(neighs,array,len,g,&x1,&y1,&x2,&y2);
    
    thislen=0;
    istext=0;
    isnot=0;
    thisarea=0;

    for(ww=0;ww<len;ww++)
      if(neighs[ww].group==g){
	temp=&(array[neighs[ww].num]->data);
	
	thisarea+=temp->set;
	
	for(tt=0;tt<num_tags;tt++){
	  if((temp->xpos>=_x1[tt]) && 
	     (temp->xpos+temp->w<_x2[tt]) &&
	     (temp->ypos>=_y1[tt]) &&
	     (temp->ypos+temp->h<_y2[tt])){
	    if(_tags[tt]==0){
	      istext++;
	    } else {
	      isnot++;
	    }
	  }
	}
      }
    
    if(fp==NULL){
      sprintf(tag,"?");
    } else if((istext>isnot) ){
      sprintf(tag,"text");
    } else if((isnot>istext) ){
      sprintf(tag,"drawing");
    } else {
      sprintf(tag,"?");
    }      

    if(thisarea*1.0/((x2-x1+1)*(y2-y1+1))>0.10){
      if((globals.g_dump_zones)){
	fprintf(stdout,"%03d %s %d %d %d %d\n",g,tag,x1,y1,x2,y2);
      }
    }
  }
}

marklistptr
groups_to_marklist(marklistptr list,
		   neighbourtype *neighs,
		   int len,
		   int number_groups,
		   double skew,
		   double within)
{
  int i,j,g,gi;
  marklistptr ret=NULL,step,temp,temp2;
  marklistptr *array;
  marktype copy;
  marktype im,im2;
  coord2d *p;

  if(len==0)
    return NULL;

  marklist_reconstruct_only_wh(list,&im);

  marktype_alloc(&im2,im.w,im.h);

  CALLOC(p,number_groups,coord2d);

  CALLOC(array,len,marklistptr);
  for(i=0,step=list;step;i++,step=step->next){
    array[i]=step;
  }

  for(g=0;g<number_groups;g++){
    int ww,thislen;
    p[g].group=g;
    if(globals.g_dump_zones){
      group_bounds_centroids(neighs,array,len,g,&p[g].x1,&p[g].y1,&p[g].x2,&p[g].y2);
    }
    else
      group_bounds(neighs,array,len,g,&p[g].x1,&p[g].y1,&p[g].x2,&p[g].y2);


    p[g].xcen=(p[g].x1+p[g].x2)/2;
    p[g].ycen=(p[g].y1+p[g].y2)/2;
  }

  if(globals.g_dump_zones){
    dump_groups_with_class(neighs,array,len,number_groups);
  }

  /*  marktype_writenamed("a.pbm",im2);*/
  marktype_free(&im2);
  /*fprintf(stderr,"written a.pbm\n");*/
  if(globals.g_dump_zones){
    exit(0);
  }

  qsort((void*)p,(unsigned int)number_groups,sizeof(coord2d),compareycoords);
  


  for(gi=0;gi<number_groups;gi++) {
    g=p[gi].group;
    temp=NULL;
    for(i=0;i<len;i++) {
      if(neighs[i].group==g){
	copy=array[neighs[i].num]->data;
	assert(copy.xpos>=p[gi].x1);
	assert(copy.ypos>=p[gi].y1);
	assert(copy.xpos<=p[gi].x2);
	assert(copy.ypos<=p[gi].y2);
	copy.grp=g;
	if(temp==NULL){
	  step=marklist_addcopy(&temp,copy);
	} else {
	  step=marklist_addcopy(&step,copy);
	}
      }
    }

    /*    fprintf(stderr,"group: %d (%d %d %d %d) == %d\n",p[gi].group,p[gi].x1,p[gi].y1,p[gi].x2,p[gi].y2,marklist_length(temp));*/

    globals.g_reading_parameter=within;        

    /*    fprintf(stderr,"within: %g\n",within);*/

    temp2=sortmarks_howard(temp,im.imagew,im.imageh,DEG2RAD(skew),300);

    /*    for(step=temp2;step;step=step->next){
      fprintf(stderr,"group: %d\n",step->data.grp);
      marktype_writeascii(stderr,step->data);
    }*/

    marklist_free(&temp);
    marklist_append(&ret,temp2);
    marklist_free(&temp2);
  }

  FREE(array);
  return ret;

}

