/*
 * Module: codebook
 *
 * Implements the routines for handling equivalance classes
 * and adding/averaging the codebook.
 *
 * The codebook module matches against the average mark example.
 * We use a 21% XOR template matcher. This works well for lossless
 * compression, but not very well for lossy compression.
 *
 * For lossless compression, the template is close enough to give
 * the contexts some reliability. For lossy compression, a CSIS
 * style template matcher should be employed.
 *
 * The codebook_configure() routine can be used to change the
 * default behaviour of the codebook. The default values
 * are an unlimited number of equivalence classes, and at most
 * 10 examples in each class.
 *
 * Author:
 *   Stuart Inglis (singlis@internz.co.nz)
 *   (c) 1998 
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "marklist.h"
#include "pbmtools.h"
#include "template_match.h"

#include "globals.h"
#include "codebook.h"


int SCALE=256;

/* the matching function uses globals.g_match_threshold */


/******************************************************************
 *                      Equivalence class
 *                           Routines
 *
 *
 ******************************************************************/

static void 
equiv_classes_init(equiv_classes *e, int max_examples)
{
  assert(e);

  if((globals.g_match_mode==MATCH_B_SINGLE)||
     (globals.g_match_mode==MATCH_F_SINGLE)||
     (globals.g_match_mode==MATCH_HYBRID)){
    e->max_examples=SCALE-1;
  } else {
    e->max_examples=max_examples;
  }
  e->num=0;
  e->set=NULL;
  marktype_init(&(e->avg));
  e->avg.bitmap=NULL;
  e->avg_uptodate=1;
  e->time=0;
}

static void
equiv_classes_free(equiv_classes *e)
{
  int i;
  
  assert(e);

  for(i=0;i<e->num;i++){
    marktype_free(&(e->set[i]));
  }
  marktype_free(&(e->avg));
  free(e->set);
}


static int 
equiv_classes_add(equiv_classes *e, marktype *d)
{
  int num,i;
  
  assert(e);
  assert(d);

  num=e->num;
  if((e->max_examples<1) || (num<e->max_examples)){
    e->set=(marktype*)realloc(e->set,sizeof(marktype)*(num+1));
    assert(e->set);
    e->set[num]=marktype_copy(*d);
    e->avg_uptodate=0;
    e->num++;
  } else {
    /* get rid of the oldest example */
    marktype_free(&(e->set[0]));
    for(i=1;i<num;i++)
      e->set[i-1]=e->set[i];
    e->set[num-1]=marktype_copy(*d);
    e->avg_uptodate=0;
    num=num-1;
  }
  
  return num;
}


/*
static void 
equiv_classes_dump(equiv_classes *e, FILE *fp)
{
  int i;
  
  assert(e);
  assert(fp);
  
  for(i=0;i<e->num;i++){
    marktype_writeascii(fp,e->set[i]);
    fprintf(fp,"crc: %d\n",marktype_crc(&(e->set[i])));
  }
}
*/

/*
static int 
equiv_classes_match(equiv_classes *e, marktype *mark, int *bestscore)
{
  int i;
  int score,found=-2;

  assert(e);
  assert(mark);
  assert(bestscore);

  *bestscore=-1;
  
  for(i=0;i<e->num;i++){
    if(NOT_SCREENED(*mark,e->set[i])){
      score=TEMP_FUNC(mark,&(e->set[i]));
      if(score<XORTHRESHOLD){
	if((*bestscore<0) || (score<*bestscore)){
	  *bestscore=score;
	  found=i;
	}
      }
    }
  }
  return found;
}
*/


static int 
equiv_classes_avg_match(equiv_classes *e, marktype *mark, int *bestscore)
{
  int i;
  int score,found=-2,score2;
  int total=0,matched=0;

  assert(e);
  assert(mark);
  assert(bestscore);

  *bestscore=-1;
  for(i=0;i<e->num;i++){
    if(NOT_SCREENED(*mark,e->set[i])){
      score=TEMP_FUNC(mark,&(e->set[i]));
      if(score<=globals.g_match_threshold){
	total+=score;
	matched+=1;
	found=1;
	/*break;*/
      }
    }
  }
  if(found>=0)
    *bestscore=(total/matched);
  return found;
}

/******************************************************************
 *                           Codebook
 *                           Routines
 *
 *
 ******************************************************************/
void
codebook_configure(codebook *c, int max_classes, int max_examples)
{
  assert(c);

  c->max_classes=max_classes;
  c->max_examples=max_examples;
}


void 
codebook_init(codebook *c, int max_classes, int max_examples)
{
  assert(c);

  c->num_eqv=0;
  c->eqv=NULL;
  c->time=0;
  codebook_configure(c,max_classes, max_examples);
}

void
codebook_updatetime(codebook *c)
{
  c->time++;
}

void 
codebook_free(codebook *c)
{
  int i;

  assert(c);

  if(c->eqv){
    for(i=0;i<c->num_eqv;i++)
      equiv_classes_free(&(c->eqv[i]));

    free(c->eqv);
  }
}

int 
codebook_add_equiv(codebook *c)
{
  int num,i;

  assert(c);
  
  num=c->num_eqv;
  if((c->max_classes<1) || (num<c->max_classes)){
    c->eqv=(equiv_classes*)realloc(c->eqv,sizeof(equiv_classes)*(num+1));
    assert(c->eqv);
    equiv_classes_init(&(c->eqv[num]),c->max_examples);
    c->num_eqv++;
  } else {
    int min,max;
    int minpos=0;
    min=max=c->eqv[0].time;
    /* getting rid of the oldest isn't very good */
    /* getting rid of the smallest is much better...*/
    /* now we get rid of the marks using LRU */
    for(i=0;i<num;i++){
      if(c->eqv[i].time<min){
	min=c->eqv[i].time;
	minpos=i;
      }
      if(c->eqv[i].time>max)
	max=c->eqv[i].time;
    }
    /*    fprintf(stderr,"deleting class %d min=%d,max=%d\n",minpos,min,max);*/
    equiv_classes_free(&(c->eqv[minpos]));
    for(i=minpos+1;i<num;i++)
      c->eqv[i-1]=c->eqv[i];
    equiv_classes_init(&(c->eqv[num-1]),c->max_examples);
    num=num-1;
  }
  c->eqv[num].time=c->time;
  return num;
}




int 
codebook_add_equiv2(codebook *c)
{
  int num,i;

  assert(c);
  
  num=c->num_eqv;
  if((c->max_classes<1) || (num<c->max_classes)){
    c->eqv=(equiv_classes*)realloc(c->eqv,sizeof(equiv_classes)*(num+1));
    assert(c->eqv);
    equiv_classes_init(&(c->eqv[num]),c->max_examples);
    c->num_eqv++;
  } else {
    int min,max;
    int minpos=0;
    min=max=c->eqv[0].time;
    /* getting rid of the oldest isn't very good */
    /* getting rid of the smallest is much better...*/
    /* now we get rid of the marks using LRU */
    for(i=0;i<num;i++){
      if(c->eqv[i].time<min){
	min=c->eqv[i].time;
	minpos=i;
      }
      if(c->eqv[i].time>max)
	max=c->eqv[i].time;
    }
    /*    fprintf(stderr,"deleting class %d min=%d,max=%d\n",minpos,min,max);*/
    equiv_classes_free(&(c->eqv[minpos]));
    for(i=minpos+1;i<num;i++)
      c->eqv[i-1]=c->eqv[i];
    equiv_classes_init(&(c->eqv[num-1]),c->max_examples);
    num=num-1;
  }
  c->eqv[num].time=c->time;
  return num*SCALE;
}


marktype *
codebook_get_avg_mark(codebook *c, int eq, int updatetimeaccess)
{
  assert(c);
  assert((eq>=0) && (eq<c->num_eqv));

  if(c->eqv[eq].avg_uptodate==0){
    if(c->eqv[eq].avg.bitmap!=NULL)
      marktype_free(&(c->eqv[eq].avg));
    equiv_classes_average_mark(&(c->eqv[eq]),&(c->eqv[eq].avg));
    c->eqv[eq].avg_uptodate=1;
  }
  if(updatetimeaccess==UPDATETIME){
    c->eqv[eq].time=c->time;
  }

  return &(c->eqv[eq].avg);
}



/* the "2" series of functions don't match with respect to the average, but
   with respect to the best fitting component over all. To keep compatible
   with the other functions we map the equivalence class along with it's
   number in the class to an integer using n=class*SCALE + pos; */
marktype *
codebook_get_avg_mark2(codebook *c, int eq, int updatetimeaccess)
{
  int i,j;
  
  assert(c);

  i=eq/SCALE;
  j=eq%SCALE;

  if(updatetimeaccess==UPDATETIME){
    c->eqv[i].time=c->time;
  }

  return &(c->eqv[i].set[j]);
}

int 
codebook_add_mark(codebook *c,int eq,marktype *mark)
{
  int n;

  assert(c);
  assert((eq>=0) && (eq<c->num_eqv));
  assert(mark);

  n=equiv_classes_add(&(c->eqv[eq]), mark);
  return n;
}

int 
codebook_add_mark2(codebook *c,int eq,marktype *mark)
{
  int n;

  assert(c);
  assert(mark);

  /* find out the class */
  eq=eq/SCALE;

  /*assert(c->eqv[eq].num<SCALE);*/
  
  n=equiv_classes_add(&(c->eqv[eq]), mark);
  return n;
}
  

int 
codebook_match(codebook *c,marktype *mark)
{
  int i;
  int found=-1,ret;
  int score;
  int bestscore=-1;
  
  assert(c);
  assert(mark);

  for(i=0;i<c->num_eqv;i++){
    /*    ret=equiv_classes_match(&(c->eqv[i]),mark, &score);*/
    ret=equiv_classes_avg_match(&(c->eqv[i]),mark, &score);

    if(ret>=0){
      if((bestscore<0)||(score<bestscore)){
	bestscore=score;
	found=i;
      }
    }
  }
  return found;
}

int 
codebook_match_against_average(codebook *c,marktype *mark)
{
  int i;
  int found=-1;
  int score,score2;
  int bestscore=-1;
  marktype *avg=NULL;
  
  assert(c);
  assert(mark);

  for(i=0;i<c->num_eqv;i++){
    avg=codebook_get_avg_mark(c,i, NO_UPDATE);
    if(NOT_SCREENED(*avg,*mark)){
      score=TEMP_FUNC(avg,mark);
      if(score<=globals.g_match_threshold){
	if((bestscore<0)||(score<bestscore)){
	  bestscore=score;
	  found=i;
	  
	  c->eqv[i].time=c->time;

	  /* if we've already found the best possible */
	  if(bestscore==0) 
	    break;

	  if((globals.g_match_mode==MATCH_F_SINGLE)||
	     (globals.g_match_mode==MATCH_F_AVG))
	    break;
	}
      }
    }
  }
  return found;
}

int 
codebook_match_against_average2(codebook *c,marktype *mark)
{
  int i,j;
  int found=-1;
  int score,score2;
  int bestscore=-1;
  marktype *avg=NULL;
  
  assert(c);
  assert(mark);

  for(i=0;i<c->num_eqv;i++){
    for(j=0;j<c->eqv[i].num;j++){
      avg=&(c->eqv[i].set[j]);
      if(NOT_SCREENED(*avg,*mark)){
	score=TEMP_FUNC(avg,mark);
	if(score<=globals.g_match_threshold){
	  if((bestscore<0)||(score<bestscore)){
	    bestscore=score;
	    found=i*SCALE + j;

	    c->eqv[i].time=c->time;

	    /* if we've already found the best possible */
	    if(bestscore==0) 
	      break;

	    if((globals.g_match_mode==MATCH_F_SINGLE)||
	       (globals.g_match_mode==MATCH_F_AVG))
	      break;
	  }
	}
      }
    }
  }
  return found;
}



void 
codebook_dump(codebook *c, int all)
{
  int i,j;

  assert(c);

  for(i=0;i<c->num_eqv;i++){
    fprintf(stderr,"********** %d: %d ******************\n",i,c->eqv[i].num);
    if(all){
      for(j=0;j<c->eqv[i].num;j++)
	marktype_writeascii(stderr,c->eqv[i].set[j]);
    }
  }
}

void 
codebook_dump_pbm(codebook *c, char *fn)
{
  int i,j;
  int MM=3000;
  int gapx=1,gapy=1;
  int MAXh,h;
  int MAXw,w;
  marktype image;
  marktype *avg=NULL;

  assert(c);
  assert(fn);

  MAXw=0;
  MAXh=0;
  for(i=0;i<c->num_eqv;i++){
    w=0;
    h=0;

    avg=codebook_get_avg_mark(c,i, NO_UPDATE);
    w+=avg->w+gapx;
    
    for(j=0;j<c->eqv[i].num;j++){
      if(c->eqv[i].set[j].h>h)
	h=c->eqv[i].set[j].h;
      w+=c->eqv[i].set[j].w;
      w+=gapx;
      if(w>MAXw)
	MAXw=w;
      if((w>=MM) && (j<c->eqv[i].num-1)){
	w=0;
	MAXh+=h;
      }
    }
    MAXh+=h;
    MAXh+=gapy;
  }

  marktype_alloc(&image,MAXw,MAXh);
  MAXw=0;
  MAXh=0;
  for(i=0;i<c->num_eqv;i++){
    w=0;
    h=0;

    avg=codebook_get_avg_mark(c,i, NO_UPDATE);
    marktype_placeat(image,*avg,w,MAXh);
    w+=avg->w+gapx;

    for(j=0;j<c->eqv[i].num;j++){
      marktype_placeat(image,c->eqv[i].set[j],w,MAXh);
      if(c->eqv[i].set[j].h>h)
	h=c->eqv[i].set[j].h;
      w+=c->eqv[i].set[j].w;
      w+=gapx;
      if(w>MAXw)
	MAXw=w;
      if((w>=MM) && (j<c->eqv[i].num-1)){
	w=0;
	MAXh+=h;
      }
    }
    MAXh+=h;
    MAXh+=gapy;
  }
  marktype_writenamed(fn,image);  
  marktype_free(&image);
}




void 
equiv_classes_average_mark(equiv_classes *e, marktype *mark)
{
  int w = 0, h = 0;
  int t = INT_MAX, b = 0, l = INT_MAX, r = 0, matched = 0;
  marktype d, *temp=NULL;
  int **a=NULL;
  int x, y,i;
  int set;

  assert(e);
  assert(mark);

  for(i=0;i<e->num;i++){
    matched++;
    temp = &(e->set[i]);
    w = MAX (w, temp->w);
    h = MAX (h, temp->h);
    r = MAX (r, temp->xcen);
    l = MIN (l, temp->xcen);
    b = MAX (b, temp->ycen);
    t = MIN (t, temp->ycen);
  }
  w = w + (r - l);
  h = h + (b - t);

  if (matched > 1) {
    int x_off, y_off;

    CALLOC_2D(a,w,h,int);
    assert(a);

    for (x = 0; x < w; x++)
      for (y = 0; y < h; y++)
	a[x][y] = 0;

    for(i=0;i<e->num;i++){
      temp=&(e->set[i]);
      x_off = r - temp->xcen;
      y_off = b - temp->ycen;
      for (x = 0; x < temp->w; x++)
	for (y = 0; y < temp->h; y++)
	  if (pbm_getpixel (temp->bitmap, x, y)){
	    assert(x+x_off>=0);
	    assert(x+x_off<w);
	    assert(y+y_off>=0);
	    assert(y+y_off<h);
	    a[x + x_off][y + y_off]++;
	  }
    }
    
    set = 0;
    d.w = w;
    d.h = h;
    marktype_alloc (&d, w, h);
    for (x = 0; x < w; x++)
      for (y = 0; y < h; y++){
	/* > gives better CR than >= */
	if (a[x][y] > MAX (1, (matched / 2))) {		
	  pbm_putpixel (d.bitmap, x, y, 1);
	  set++;
	}
      }
    FREE_2D(a,w);
    marktype_area (&d);
    assert(d.set!=0);
    marktype_adj_bound (&d);
    d.set = set;
    d.name = (char *) realloc (d.name, sizeof (char) * (strlen ("?") + 1));
    assert(d.name);
    strcpy (d.name, "?");
    marktype_calc_centroid (&d);
  } else
    d = marktype_copy (e->set[0]);

  *mark = d;
}

