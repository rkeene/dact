

#include "line.h"
#include "boundary.h"

#include "globals.h"

void
line_init(linetype *z)
{
  z->num_marks=0;
  z->mark=NULL;
}

void 
line_add(linetype *z, marklistptr list)
{
  int i=0;
  marklistptr step;

  z->num_marks=marklist_length(list);
  z->mark=(marktype*)realloc(z->mark,z->num_marks*sizeof(marktype));

  for(i=0,step=list;step; i++,step=step->next){
    z->mark[i]=(step->data);
  }
}

int 
line_stats(linetype *z)
{
  float xi=0,yi=0,xiyi=0,xi2=0,tx,ty,a,b;
  int n=0,j;
  
  if(z->num_marks<0)
    return 0;
  
  assert(z->num_marks!=0);

  for(j=0;j<z->num_marks;j++){
    n++;
    xi+=(tx=(z->mark[j].xpos));
    yi+=(ty=(z->mark[j].ypos+z->mark[j].h-1));
    xi2+=(tx*tx);
    xiyi+=(tx*ty);
  }
  b=xiyi - (xi*yi/n);
  b=b/(xi2 - (xi*xi/n));
  
  a=yi/n - b*(xi/n);

  /*  fprintf(stderr,"%.2f %.2f\n",a,b);*/
  if(n>10)
    return (int)(yi/n);
  else
    return z->mark[0].ypos+z->mark[0].h-1;
}

void
line_stats_two(linetype *z, int *a, float *b)
{
  float xi=0,yi=0,xiyi=0,xi2=0,tx,ty;
  int n=0,j;
  
  if(z->num_marks<0){
    *a=0;
    *b=0;
  }
  
  for(j=0;j<z->num_marks;j++){
    n++;
    xi+=(tx=(z->mark[j].xpos));
    yi+=(ty=(z->mark[j].ypos+z->mark[j].h-1));
    xi2+=(tx*tx);
    xiyi+=(tx*ty);
  }
  *b=xiyi - (xi*yi/n);
  *b=(*b)/(xi2 - (xi*xi/n));
  
  *a=(int)(yi/n - (*b)*(xi/n) + 0.5);
}

int
line_stats_indirect(linetype *z, linetype *indirect)
{
  float xi=0,yi=0,xiyi=0,xi2=0,tx,ty,a,b;
  int n=0,j;
  int pos;

  if(z->num_marks<0)
    return 0;  

  assert(z->num_marks!=0);
  
  for(j=0;j<z->num_marks;j++){
    pos=z->mark[j].symnum;

    n++;
    xi+=(tx=(indirect->mark[pos].xpos));
    yi+=(ty=(indirect->mark[pos].ypos+indirect->mark[pos].h-1));
    xi2+=(tx*tx);
    xiyi+=(tx*ty);
  }
  b=xiyi - (xi*yi/n);
  b=b/(xi2 - (xi*xi/n));
  /* y increases downwards */
  b=-b;
  
  a=yi/n - b*(xi/n);

  /*  fprintf(stderr,"%d: %.2f %.2f %.2f\n",n,a,b,RAD2DEG(atan(b)));*/
  if(n>10)
    return (int)(yi/n);
  else
    return z->mark[0].ypos+z->mark[0].h-1;
}

static int CmpOnXpos(const void *e1, const void *e2)
{
    return ( (((marktype*)e1)->xpos) - (((marktype*)e2)->xpos) );
}

static int CmpOnXcen(const void *e1, const void *e2)
{
    return ( (((marktype*)e1)->xpos+((marktype*)e1)->xcen)  
	    - (((marktype*)e2)->xpos+((marktype*)e2)->xcen));
}

void
line_sort_xpos(linetype *z)
{
  qsort((void*)z->mark,(unsigned int)z->num_marks,sizeof(marktype),CmpOnXpos);
}

void
line_sort_xcen(linetype *z)
{
  qsort((void*)z->mark,(unsigned int)z->num_marks,sizeof(marktype),CmpOnXcen);
}

  


void
line_addmark(linetype *z, marktype *m)
{
  int n;
  n=z->num_marks;

  z->mark=(marktype*)realloc(z->mark,(z->num_marks+1)*sizeof(marktype));
  z->mark[z->num_marks]=(*m);
  z->num_marks++;
}



void
line_free(linetype *z)
{
  int i;
  for(i=0;i<z->num_marks;i++){
    /*marktype_free(&z->mark[i]);*/
  }
  free(z->mark);
}

marklistptr
lines_to_listcopy(linetype **lines, int num_lines)
{
  marklistptr list=NULL,step;
  int j,i;
  marktype copy;

  for(j=0;j<num_lines;j++){
    
    for(i=0;i< (*lines)[j].num_marks;i++){
      copy=(*lines)[j].mark[i];
      if((i==0) && ((*lines)[j].num_marks>=2)){
	copy.start_of_line=START_OF_LINE; 
	copy.ypos_of_line=line_stats(&(*lines)[j]);
      } else if (i==(*lines)[j].num_marks-1){
	copy.start_of_line=END_OF_LINE;
      } else {
	copy.start_of_line=0;
      }

      if(list==NULL){
	step=marklist_addcopy(&list,copy);
      } else {
	step=marklist_addcopy(&step,copy);
      }
    }
  }
  return list;
}


marklistptr
lines_to_listcopy_indirect(linetype **lines, linetype *indirect,int num_lines)
{
  marklistptr list=NULL,step;
  int j,i;
  int li=0,lj=0;
  marktype copy;
  marktype space;

  marktype_init(&space);
  space.start_of_line=SPACE;

  for(j=0;j<num_lines;j++){
    
    for(i=0;i< (*lines)[j].num_marks;i++){

      copy=indirect->mark[ (*lines)[j].mark[i].symnum ];

      copy.start_of_line=0;

      if((i==0) && ((*lines)[j].num_marks>=2)){
	copy.start_of_line=START_OF_LINE; 
	line_stats_two(&(*lines)[j], &copy.ypos_of_line,&copy.skew_of_line);
      } else if (i==(*lines)[j].num_marks-1) {
	copy.start_of_line=END_OF_LINE;
      }

      if((copy.start_of_line==END_OF_LINE) || (copy.start_of_line==0)){
	if(globals.g_insert_spaces){
	  if ( ((copy.xpos+copy.xcen-li)>1.3*globals.g_reading_parameter) 
	       ){
	    space.xpos=li;
	    space.ypos=lj;
	    space.w=((copy.xpos+copy.xcen)-li);
	    /*(int)(globals.g_reading_parameter);*/
	    if(list==NULL){
	      step=marklist_addcopy(&list,space);
	    } else {
	      step=marklist_addcopy(&step,space);
	    }
	  } 
	}
      }

      if(list==NULL){
	step=marklist_addcopy(&list,copy);
      } else {
	step=marklist_addcopy(&step,copy);
      }
      li=copy.xpos+copy.xcen;
      lj=copy.ypos+copy.h-1;
    }
  }
  return list;
}





void
line_test()
{
  marktype im;
  marklistptr list=NULL;
  linetype z;

  marktype_readnamed("b.pbm",&im);
  list=extract_all_marks(list,im,1,8);

  line_init(&z);
  line_add(&z,list);
  line_free(&z);

  fprintf(stderr,"line_test() finished\n");
}




