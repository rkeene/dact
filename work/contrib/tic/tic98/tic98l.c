/*
 * Module: tic98
 * 
 *
 * This module implements lossless textual image compression, using
 * adaptive library encoding and averaging.
 *
 * These routines take a marklist, and encoding the marks using an
 * adaptive Mohiuddin/SPM style coding.  I have extended the model to
 * handle equivalence classes and example lists, with optional pruning.
 *
 * The codebook is initialized in tic98_init, with the maximum number
 * of classes set to DEFAULT_MAX_CLASSES, and the maximum number of
 * examples in each class set to DEFAULT_MAX_EXAMPLES. Typical values
 * of these two parameters would be UNLIMITED(-1) and 10.  The classes
 * are pruning in a LRU ordering, while the examples are pruned in
 * FIFO order.
 *
 * Integers are coding using the b_gamma() routines which form a
 * three-stage binary tree. The path from the root to the leaves forms
 * a gamma encoding.
 *
 * Marks that are matched are coding clairvoyantly with respect to the
 * average mark in the class.
 *
 * The offsets are calculated using the Witten, Inglis, et. al, method
 * from Proc. IEEE in 1994. This needs to be brought up to date.  (An
 * offset is in fact coding twice, the sequence 2,0,2,0 is coded
 * 2,-2,2,-2 while knowing the values should be around two requires
 * less coding... 0,-2,0,-2)
 *
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

#include "globals.h"

#include "tic98.h"
#include "arithcode.h"

#include "marklist.h"
#include "line.h"

#include "codebook.h"
#include "context.h"

#include "b_gamma.h"

#include "ppmd_model.h"


void
tic98_init(tic98type *t)
{
  t->misc=INTCODE_INIT();
  t->cx=INTCODE_INIT();
  t->cy=INTCODE_INIT();
  t->z_w=INTCODE_INIT();
  t->z_h=INTCODE_INIT();
  t->z_rw=INTCODE_INIT();
  t->z_rh=INTCODE_INIT();
  t->p_cl=INTCODE_INIT();
  t->xpos=INTCODE_INIT();
  t->ypos=INTCODE_INIT();

  codebook_init(&(t->c),
		globals.g_max_classes,
		globals.g_max_examples);
  context_init(&(t->global),JBIG_SIZE);
  context_init(&(t->clairall),JBIG_CLAIR_SIZE);
}


void
tic98_free(tic98type *t)
{
  assert(t);

  INTCODE_FREE(t->misc);
  INTCODE_FREE(t->cx);
  INTCODE_FREE(t->cy);
  INTCODE_FREE(t->z_w);
  INTCODE_FREE(t->z_h);
  INTCODE_FREE(t->z_rw);
  INTCODE_FREE(t->z_rh);
  INTCODE_FREE(t->p_cl);
  INTCODE_FREE(t->xpos);
  INTCODE_FREE(t->ypos);

  codebook_free(&(t->c));

  context_free(&(t->global));
  context_free(&(t->clairall));
}


void 
tic98_num_encode(tic98type *t, int num)
{
  INTCODE_ENCODE(t->misc,num);
}

int
tic98_num_decode(tic98type *t)
{
  return INTCODE_DECODE(t->misc);
}


void
tic98_start_encoding(void)
{
  START_ENC();
}

void
tic98_finish_encoding(void)
{
  FINISH_ENC();
}


void
tic98_start_decoding(void)
{
  START_DEC();
}

void
tic98_finish_decoding(void)
{
  FINISH_DEC();
}

void 
tic98_status(tic98type *t, FILE *fp)
{
  int tot,i,min,max,n;
  
  if(fp==NULL)
    return;


  tot=0;
  n=t->c.num_eqv;
  if(n)
    min=max=t->c.eqv[0].num;
  else 
    min=max=0;
  for(tot=0,i=0; i<t->c.num_eqv ;i++){
    tot+=t->c.eqv[i].num;
    if(t->c.eqv[i].num>max) max=t->c.eqv[i].num;
    if(t->c.eqv[i].num<min) min=t->c.eqv[i].num;
  }
  fprintf(fp,
	  "bits:        %ld\n"
	  "bytes:       %ld\n"
	  "Num_marks:   %d\n"
	  "Num_matched: %d\n"
	  "Percentage_matched: %.1f\n"
	  "Singletons:  %d\n"
	  "Percentage_singletons:  %.1f\n"
	  "Min_marks_in_class: %d\n"
	  "Avg_marks_in_class: %.1f\n"
	  "Max_marks_in_class: %d\n",

	  CountOfBitsOut,
	  (long)(ceil(CountOfBitsOut/8.0)),
	  tot,
	  globals.g_num_matched,
	  globals.g_num_matched*100.0/tot,
	  tot-globals.g_num_matched,
	  (tot-globals.g_num_matched)*100.0/tot,
	  min,(n==0)?0:tot*1.0/n,max
	  );
  
  fprintf(fp,"Reading_order: ");
  switch (globals.g_reading_order){
  case EXTRACTION_ORDER: fprintf(fp,"Extraction");break;
  case EXTRACTION_PROFILE: fprintf(fp,"Profile");break;
  case EXTRACTION_RANDOM: fprintf(fp,"Random");break;
  case EXTRACTION_HOWARD: fprintf(fp,"Howard");break;
  case EXTRACTION_ZHANG: fprintf(fp,"Zhang");break;
  case EXTRACTION_DOCSTRUM: fprintf(fp,"Docstrum");break;
  default:
    fprintf(stderr,"reading order not defined\n");
    abort();
  }
  fprintf(fp," %g\n",globals.g_reading_parameter);
  
  fprintf(fp,"Num_classes: %d\n",t->c.num_eqv);
  fflush(fp);
}


void 
tic98_compress_marks(tic98type *t, marklistptr list, int _w, int _h)
{
  marklistptr step=NULL;
  int rh,rw;
  int i;
  int len,index;
  int lastx=0,lasty=0,xoff=0,yoff=0;
  int carryx=0,carryy=0;
  int currentypos=0, predicty;
  float lineskew=0;
  marklistptr recon=NULL;
  marktype temp;

  assert(t);
  
  len=marklist_length(list);
      
  INTCODE_ENCODE (t->z_w,len);
  INTCODE_ENCODE (t->z_w,_w);
  INTCODE_ENCODE (t->z_w,_h);

  /* +1 because we add one later on */
  /*
  init_ppmd_globals();
  ppm_start_encoding (3, 2*len);*/



  i=0;
  for(step=list;step;step=step->next){
    /*    fprintf(stderr,"%d/%d %d (%d %d)\n",i,len-1,step->data.grp,step->data.xpos,step->data.ypos+step->data.h-1);*/
    /*    marktype_writeascii(stderr,step->data);*/

    codebook_updatetime(&(t->c));
    index=codebook_match_against_average(&(t->c),&(step->data));
    assert(index<t->c.num_eqv);

    if(step->data.start_of_line==START_OF_LINE){
      currentypos=step->data.ypos_of_line;
      lineskew=step->data.skew_of_line;
    }

    INTCODE_ENCODE (t->p_cl,index);
    /*    ppm_encode_symbol(index+1); */


    if(index>=0){
      marktype *avg;
      int dx,dy;
      avg=codebook_get_avg_mark(&(t->c),index, UPDATETIME);

      marktype_calc_centroid(avg);
      dx=step->data.xcen - avg->xcen;
      dy=step->data.ycen - avg->ycen;

      INTCODE_ENCODE (t->xpos,dx+step->data.xpos - lastx);
      INTCODE_ENCODE (t->ypos,dy+step->data.ypos - lasty);

      globals.g_num_matched++;

      lastx=dx + step->data.xpos;
      lasty=dy + step->data.ypos; 

      /**/
      temp=*avg;
      temp.xpos=lastx;
      temp.ypos=lasty;
      marklist_addcopy(&recon,temp);

    } else {
      INTCODE_ENCODE (t->z_w,step->data.w);
      INTCODE_ENCODE (t->z_h,step->data.h);

      INTCODE_ENCODE (t->xpos,step->data.xpos - lastx);
      INTCODE_ENCODE (t->ypos,step->data.ypos - lasty);

      compress_mark(&(t->global),&(step->data), COMPRESS);
      index=codebook_add_equiv(&(t->c));
      codebook_add_mark(&(t->c),index,&(step->data));

      lastx=step->data.xpos;
      lasty=step->data.ypos;

      /**/
      temp=step->data;
      temp.xpos=lastx;
      temp.ypos=lasty;

      marklist_addcopy(&recon,temp);
    }

    i++;
  }
  {
    marktype im_r,im;
    int i,j;

    marklist_reconstruct(recon,&im_r);
    marktype_writenamed("recon.pbm",im_r);

    marklist_reconstruct(list,&im);
    marktype_writenamed("image.pbm",im);

    {
      int num=0;
      int li=0,lj=0;
      int p1,p2;

      for(j=0;j<im.h;j++){
	for(i=0;i<im.w;i++){
	  p1=gpix(im,i,j);
	  p2=gpix(im_r,i,j);
	  if(p1!=p2)
	    num++;
	}
      }
      /*      fprintf(stderr,"%d\n",num);*/
	  
      for(j=0;j<im.h;j++){
	for(i=0;i<im.w;i++){
	  p1=gpix(im,i,j);
	  p2=gpix(im_r,i,j);
	  if(p1!=p2){
	    /*INTCODE_ENCODE (t->xpos,i -li);
	    INTCODE_ENCODE (t->ypos,j -lj);*/
	    /*fprintf(stderr,"%d %d p\n",i,j);*/
	    li=i;
	    lj=j;
	  }
	}
      }
    }
    marktype_free(&im_r);
    marktype_free(&im);

  }



}


marklistptr tic98_decompress_marks(tic98type *t, int *_w, int *_h)
{
  int w,h;
  marktype mark;
  marklistptr list=NULL,step=NULL;
  int rh,rw;
  int i;
  int len,index;
  int lastx=0,lasty=0,xoff=0,yoff=0;
  int carryx=0,carryy=0;
  marktype image;
  unsigned int eof_sym;


  len=INTCODE_DECODE(t->z_w);
  *_w=image.w=INTCODE_DECODE(t->z_w);
  *_h=image.h=INTCODE_DECODE(t->z_w);

  /*  init_ppmd_globals();
  eof_sym = ppm_start_decoding (3, 2*len);*/


  for(i=0;i<len;i++){
    codebook_updatetime(&(t->c));

    index=INTCODE_DECODE(t->p_cl);

    if(index>=0){
      marktype *avg;
      avg=codebook_get_avg_mark(&(t->c),index, UPDATETIME);

      xoff=INTCODE_DECODE(t->xpos);
      yoff=INTCODE_DECODE(t->ypos);

      mark=marktype_copy(*avg);
      mark.xpos=lastx+xoff;
      mark.ypos=lasty+yoff;
    } else {
      w=INTCODE_DECODE (t->z_w);
      h=INTCODE_DECODE (t->z_h);

      xoff=INTCODE_DECODE(t->xpos);
      yoff=INTCODE_DECODE(t->ypos);

      decompress_mark(&(t->global),&mark,w,h);

      mark.xpos=lastx+xoff;
      mark.ypos=lasty+yoff;

      index=codebook_add_equiv(&(t->c));
      marktype_calc_centroid(&mark);
      codebook_add_mark(&(t->c),index,&mark);
    }
    mark.imagew=image.w;
    mark.imageh=image.h;

    if(list==NULL) step=marklist_add(&list,mark);
    else step=marklist_add(&step,mark);

    /*    marktype_writeascii(stderr,mark);*/
	  
    lastx=step->data.xpos;
    lasty=step->data.ypos;
  }

  /*  ppm_finish_decoding();*/

  return list;
}
