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

#ifndef Q_CODER
#include "ppmd_model.h"
#endif

#include "sort_lines.h"

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
  int tot,i,min,max,n,sing;
  
  if(fp==NULL)
    return;

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
  for(sing=0,i=0; i<t->c.num_eqv ;i++){
    if(t->c.eqv[i].num==1)
      sing++;
  }

  fprintf(fp,
	  "Compiled_date:       "__DATE__"\n"
	  "bits:                %ld\n"
	  "bytes:               %ld\n"
	  "Context_arity:       %d\n"
	  "JBIG_size:           %d\n"
	  "JBIG_clair_1:        %d\n"
	  "JBIG_clair_2:        %d\n"
	  "Update_probs:        %s\n"
	  "Match_mode:          %s\n"
	  "Phi:                 %d\n"
	  "Use_PPM:             %s\n"
	  "Align_using_centres: %s\n"
	  "Template_method:     %s\n"
	  "Template_thres:      %d\n"
	  "Marks_in_codebook:   %d\n"
	  "Num_marks:           %d\n"
	  "Num_matched:         %d\n"
	  "Percent_matched:     %.1f\n"
	  "Singletons:          %d\n"
	  "Percent_singletons:  %.1f %%\n"
	  "Num_classes:         %d\n"
	  "Min_marks_in_class:  %d\n"
	  "Avg_marks_in_class:  %.1f\n"
	  "Max_marks_in_class:  %d\n",

	  CountOfBitsOut,
	  (long)(ceil(CountOfBitsOut/8.0)),
	  CONTEXT_BITS,
	  JBIG_SIZE,
	  JBIG_CLAIR_1,
	  JBIG_CLAIR_2,
	  globals.g_update_probs_after_encoding?"True":"False",
	  (globals.g_match_mode==MATCH_B_AVG)?"B_AVG":
	  ((globals.g_match_mode==MATCH_F_AVG)?"F_AVG":
	   ((globals.g_match_mode==MATCH_B_SINGLE)?"B_SINGLE":
	    ((globals.g_match_mode==MATCH_F_SINGLE)?"F_SINGLE":
	     ((globals.g_match_mode==MATCH_HYBRID)?"HYBRID":"!!ERROR!!")))),
	  SCALE,
	  globals.g_use_ppm?"True":"False",
	  globals.g_align_using_centres?"True":"False",
	  TEMP_FUNC_STRING,
	  globals.g_match_threshold,
	  tot,
	  globals.g_marks_on_page,
	  globals.g_num_matched,
	  globals.g_num_matched*100.0/globals.g_marks_on_page,
	  sing,
	  sing*100.0/t->c.num_eqv,
	  t->c.num_eqv,
	  min,(n==0)?0:tot*1.0/n,max
	  );
  
  fprintf(fp,"Reading_order:       ");
  switch (globals.g_reading_order){
  case EXTRACTION_ORDER:    fprintf(fp,"Extraction");break;
  case EXTRACTION_PROFILE:  fprintf(fp,"Profile");break;
  case EXTRACTION_RANDOM:   fprintf(fp,"Random");break;
  case EXTRACTION_HOWARD:   fprintf(fp,"Howard");break;
  case EXTRACTION_ZHANG:    fprintf(fp,"Zhang");break;
  case EXTRACTION_DOCSTRUM: fprintf(fp,"Docstrum");break;
  default:
    fprintf(fp,"!!ERROR!!\n");
  }
  fprintf(fp," %g\n",globals.g_reading_parameter);
  
  fflush(fp);

}



static int CmpInt(const void *e1, const void *e2)
{
    return ( (((int*)e1)) - (((int*)e2)) );
}

void 
tic98_compress_marks(tic98type *t, marklistptr list, int _width, int _height)
{
  marklistptr step=NULL;
  int rh,rw;
  int i;
  int len,index;
  int lastx=0,lasty=0,xoff=0,yoff=0;
  int carryx=0,carryy=0;
  marktype image;
  int currentypos=0;
  float lineskew=0;
  int numpixels=0;

  assert(t);
  
  if((globals.g_match_mode==MATCH_B_AVG) || 
     (globals.g_match_mode==MATCH_F_AVG)){
    SCALE=1;
  }

  len=marklist_length(list);

  globals.g_marks_on_page=len;

  i=0;
  for(i=0,step=list;step;i++,step=step->next){
    if(step->data.start_of_line==START_PRUNED)
      break;
  }

  INTCODE_ENCODE (t->z_w,i);
  INTCODE_ENCODE (t->z_w,_width);
  INTCODE_ENCODE (t->z_w,_height);

  /* +1 because we add one later on */

#ifndef Q_CODER
  if(globals.g_use_ppm){
    init_ppmd_globals();
    ppm_start_encoding (2, len+2);
  }
#endif



  i=0;
  for(step=list;step;step=step->next){

    if(step->data.start_of_line==START_PRUNED)
      break;

    /*if((i%50)==0){
      fprintf(stderr,"%d/%d %d (%d %d)     \r",i,len-1,step->data.grp,step->data.xpos,step->data.ypos+step->data.h-1);
      }
    */

    if(step->data.start_of_line==SPACE){
      abort();
      index=len-2;
    } else {
      codebook_updatetime(&(t->c));
      if((globals.g_match_mode==MATCH_B_SINGLE) || 
	 (globals.g_match_mode==MATCH_F_SINGLE) || 
	 (globals.g_match_mode==MATCH_HYBRID)){
	index=codebook_match_against_average2(&(t->c),&(step->data));
      } else {
	index=codebook_match_against_average(&(t->c),&(step->data));
	assert(index<t->c.num_eqv);
      }
    }

    if(step->data.start_of_line==START_OF_LINE){
      currentypos=step->data.ypos_of_line;
      lineskew=step->data.skew_of_line;
    }

#ifndef Q_CODER
    if(globals.g_use_ppm){
      ppm_encode_symbol(index+1); 
    } else {
#endif
      INTCODE_ENCODE (t->p_cl,index);
#ifndef Q_CODER
    }
#endif

    if(globals.g_dump_indices){
      if(index==-1){
	if(globals.g_dump_indices==1)
	  fprintf(stderr,"%d\n",index+1);
	else
	  fprintf(stderr,"%d\n",t->c.num_eqv+1);
      } else {
	fprintf(stderr,"%d\n",index+1);
      }
    }


    if(step->data.start_of_line==SPACE){
      lastx+=10;
      continue;
    }

    xoff=step->data.xpos                                 -lastx;

    /* it's - because origin at top left */
    /*    lasty = lasty - ROUND(sin(globals.g_skew)*xoff);*/

    yoff=(step->data.ypos+step->data.h-1)               -lasty;
	
    lastx=step->data.xpos+step->data.w;
    lasty=step->data.ypos+step->data.h-1;
    
    INTCODE_ENCODE (t->xpos,xoff);
    INTCODE_ENCODE (t->ypos,yoff);

    if(index>=0){
      marktype *avg;
      int carryx1,carryy1;
      if((globals.g_match_mode==MATCH_B_SINGLE)||
	 (globals.g_match_mode==MATCH_F_SINGLE)||
	 (globals.g_match_mode==MATCH_HYBRID)){
	avg=codebook_get_avg_mark2(&(t->c),index, UPDATETIME);
      } else {
	avg=codebook_get_avg_mark(&(t->c),index, UPDATETIME);
      }
      
      globals.g_num_matched++;

      rw=step->data.w - avg->w;
      rh=step->data.h - avg->h;

      INTCODE_ENCODE (t->z_rw,rw);
      INTCODE_ENCODE (t->z_rh,rh);

      if(globals.g_align_using_centres){
	carryx=step->data.w/2 - avg->w/2;
	carryy=step->data.h/2 - avg->h/2;
      } else {
	carryx=step->data.xcen - avg->xcen;
	carryy=step->data.ycen - avg->ycen;
	
	INTCODE_ENCODE (t->cx,carryx);
	INTCODE_ENCODE (t->cy,carryy);
      }
      

      if(globals.g_match_mode==MATCH_HYBRID){
	int q;
	
	for(q=0;q<t->c.eqv[index/SCALE].num;q++){
	  compress_mark_clair(&(t->clairall),
			      &(t->c.eqv[index/SCALE].set[q]), avg,
			      (t->c.eqv[index/SCALE].set[q].xcen)-avg->xcen,
			      (t->c.eqv[index/SCALE].set[q].ycen)-avg->ycen,
			      UPDATE); 
	}
      }
	
      compress_mark_clair(&(t->clairall),&(step->data),avg,
			  carryx,carryy,
			  COMPRESS); 
	    
      if((globals.g_match_mode==MATCH_B_SINGLE) ||
	 (globals.g_match_mode==MATCH_F_SINGLE) || 
	 (globals.g_match_mode==MATCH_HYBRID)){
	codebook_add_mark2(&(t->c),index,&(step->data));
      } else {
	codebook_add_mark(&(t->c),index,&(step->data));
      }
      if(globals.g_update_probs_after_encoding){
	compress_mark(&(t->global),&(step->data),UPDATE);
      }
    } else {
      INTCODE_ENCODE (t->z_w,step->data.w);
      INTCODE_ENCODE (t->z_h,step->data.h);

      compress_mark(&(t->global),&(step->data), COMPRESS);

      if((globals.g_match_mode==MATCH_B_SINGLE) ||
	 (globals.g_match_mode==MATCH_F_SINGLE) || 
	 (globals.g_match_mode==MATCH_HYBRID)){
	index=codebook_add_equiv2(&(t->c));
	codebook_add_mark2(&(t->c),index,&(step->data));
      } else {
	index=codebook_add_equiv(&(t->c));
	codebook_add_mark(&(t->c),index,&(step->data));
      }
    }
    i++;

  }

#ifndef Q_CODER   
  if(globals.g_use_ppm){
    ppm_finish_encoding ();
  }
#endif

  {
    marklistptr step2=step;
    numpixels=0;
    for(;step2;step2=step2->next){
      numpixels+=step2->data.set;
    }
  }

  INTCODE_ENCODE (t->z_w,numpixels);

  {
    int li=0,lj=0;
    for(;step;step=step->next){
      int i,j;

      if(globals.g_dump_indices)
	fprintf(stderr,"%d\n",t->c.num_eqv+1);

      for(j=0;j<step->data.h;j++)
	for(i=0;i<step->data.w;i++){
	  if(gpix(step->data,i,j)){
	    INTCODE_ENCODE (t->xpos,step->data.xpos+i -li);
	    INTCODE_ENCODE (t->ypos,step->data.ypos+j -lj);
	    /*	    fprintf(stderr,"%d %d p\n",step->data.xpos+i,step->data.ypos+j);*/
	    li=step->data.xpos+i;
	    lj=step->data.ypos+j;
	  }
	}
    }
  }

  if(globals.g_codebook_filename){
    codebook_dump_pbm(&t->c, globals.g_codebook_filename);
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
  int numpixels;


  len=INTCODE_DECODE(t->z_w);
  *_w=image.w=INTCODE_DECODE(t->z_w);
  *_h=image.h=INTCODE_DECODE(t->z_w);

  if((globals.g_match_mode==MATCH_B_AVG) || 
     (globals.g_match_mode==MATCH_F_AVG) ||
     (globals.g_match_mode==MATCH_HYBRID)){
    SCALE=1;
  }


#ifndef Q_CODER
  if(globals.g_use_ppm){
    init_ppmd_globals();
    eof_sym = ppm_start_decoding (2, len+2);
  }
#endif


  for(i=0;i<len;i++){
    marktype *avg;
    codebook_updatetime(&(t->c));

#ifndef Q_CODER
    if(globals.g_use_ppm){
      index=(int)ppm_decode_symbol() - 1;
    } else {
#endif
      index=INTCODE_DECODE(t->p_cl);
#ifndef Q_CODER
    }
#endif
      

    xoff=INTCODE_DECODE(t->xpos);
    yoff=INTCODE_DECODE(t->ypos);

 
    lastx+=xoff;

    /*    lasty = lasty - ROUND(sin(globals.g_skew)*xoff);*/
    lasty+=yoff;

    avg=NULL;
    if(index>=0){
      if((globals.g_match_mode==MATCH_F_SINGLE) ||
	 (globals.g_match_mode==MATCH_B_SINGLE)){
	avg=codebook_get_avg_mark2(&(t->c),index, UPDATETIME);
      } else {
	avg=codebook_get_avg_mark(&(t->c),index, UPDATETIME);
      }

      rw=INTCODE_DECODE (t->z_rw);
      rh=INTCODE_DECODE (t->z_rh);

      rw+=avg->w;
      rh+=avg->h;
      
      if(globals.g_align_using_centres){
	carryx=rw/2 - avg->w/2;
	carryy=rh/2 - avg->h/2;
      } else {
	carryx=INTCODE_DECODE(t->cx);
	carryy=INTCODE_DECODE(t->cy);
      }



      if(globals.g_match_mode==MATCH_HYBRID){
	int q;
	marktype *avg_actual=codebook_get_avg_mark(&(t->c),index/SCALE, UPDATETIME);
	
	for(q=0;q<t->c.eqv[index/SCALE].num;q++){
	  compress_mark_clair(&(t->clairall),
			      &(t->c.eqv[index/SCALE].set[q]),avg_actual,
			      (t->c.eqv[index/SCALE].set[q].xcen)-avg_actual->xcen,
			      (t->c.eqv[index/SCALE].set[q].ycen)-avg_actual->ycen,
			      UPDATE); 
	}
      }

      decompress_mark_clair(&(t->clairall),&mark,avg,
			    rw,rh,
			    carryx,carryy);

      marktype_calc_centroid(&mark); 
      if((globals.g_match_mode==MATCH_F_SINGLE) ||
	 (globals.g_match_mode==MATCH_B_SINGLE)){
	codebook_add_mark2(&(t->c),index,&mark); 
      } else { 
	codebook_add_mark(&(t->c),index,&mark); 
      }
      if(globals.g_update_probs_after_encoding){
	compress_mark(&(t->global),&mark,UPDATE);
      }

    } else {
      w=INTCODE_DECODE (t->z_w);
      h=INTCODE_DECODE (t->z_h);

      decompress_mark(&(t->global),&mark,w,h);
      marktype_calc_centroid(&mark);

      if((globals.g_match_mode==MATCH_F_SINGLE) ||
	 (globals.g_match_mode==MATCH_B_SINGLE)){
	index=codebook_add_equiv2(&(t->c));
	codebook_add_mark2(&(t->c),index,&mark);
      } else {
	index=codebook_add_equiv(&(t->c));
	codebook_add_mark(&(t->c),index,&mark);
      }
    }
    mark.imagew=image.w;
    mark.imageh=image.h;
    mark.xpos=lastx;
    mark.ypos=lasty-(mark.h-1);

    if(list==NULL) step=marklist_add(&list,mark);
    else step=marklist_add(&step,mark);

    /*marktype_writeascii(stderr,mark);*/
	  
    lastx+=mark.w;
  }

#ifndef Q_CODER
  if(globals.g_use_ppm){
    ppm_finish_decoding();
  }
#endif


  numpixels=INTCODE_DECODE(t->z_w);


  {
    marktype pixel;
    int li=0,lj=0;

    marktype_alloc(&pixel,1,1);
    ppix(pixel,0,0,1);

    pixel.imagew=image.w;
    pixel.imageh=image.h;

    for(i=0;i<numpixels;i++){
      li=(pixel.xpos=INTCODE_DECODE(t->xpos)+li);
      lj=(pixel.ypos=INTCODE_DECODE(t->ypos)+lj);
      
      if(list==NULL) step=marklist_addcopy(&list,pixel);
      else step=marklist_addcopy(&step,pixel);
    }
  }


  return list;
}
