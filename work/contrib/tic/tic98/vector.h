/*****************************************************************************
 *
 *   Author:   Stuart Inglis     <singlis@waikato.ac.nz>
 *
 *
 *****************************************************************************/
#ifndef __VECTOR_H
#define __VECTOR_H

#include "marklist.h"
#include "windowing.h"

typedef struct {
  int size;
  double *v;
} vectortype;

void  vector_init(vectortype *v);
void  vector_alloc(vectortype *v, int size);
void  vector_free(vectortype *v);
void  vector_fprintf(FILE* fp, vectortype *hist);
void  vector_fprint_named(char *fn, vectortype *hist);
void  vector_image_vertical(vectortype *hist, marktype image);
void  vector_image_vertical_rles(vectortype *hist, marktype image);
void  vector_image_horizontal(vectortype *hist, marktype image);
void  vector_image_horizontal_rles(vectortype *hist, marktype image);
void  vector_norm(vectortype *hist);
void  vector_invert(vectortype *hist);
void  vector_quantise(vectortype *hist, int levels);
void  vector_threshold(vectortype *hist, double thres, double below, double above);
void  vector_convolve(vectortype *hist, int size, int TYPE);
void  vector_image_highlight_vertical(vectortype *hist, marktype image);
void  vector_image_highlight_horizontal(vectortype *hist, marktype image);
void  vector_image_plot(vectortype *hist, marktype image);
void  vector_and(vectortype *res, vectortype *anded);
void  vector_or(vectortype *res, vectortype *ored);
void  vector_mean(vectortype *hist1);
void  vector_edge(vectortype *hist1);
void  vector_stats(vectortype *hist, double *mean, double *sd);



#endif

