#ifndef _GLOBALS_H
#define _GLOBALS_H


#include <stdio.h>

#define UNLIMITED -1

#define EXTRACTION_ORDER   1
#define EXTRACTION_PROFILE 2
#define EXTRACTION_RANDOM  3
#define EXTRACTION_ZHANG   4
#define EXTRACTION_HOWARD  5
#define EXTRACTION_DOCSTRUM 6

#define MATCH_B_AVG    1
#define MATCH_F_AVG    2
#define MATCH_B_SINGLE 4
#define MATCH_F_SINGLE 8
#define MATCH_HYBRID   16



typedef struct {
  int g_reading_order;
  float g_reading_parameter;
  int g_default_resolution;
  int g_extraction_nested;
  int g_extraction_connectivity;
  int g_gamma_init;
  int g_gamma_norm;
  int g_gamma_increment;
  int g_max_classes;
  int g_max_examples;
  int g_match_threshold;
  int g_context_init;
  int g_context_norm;
  int g_context_increment;
  int g_marks_on_page;
  int g_num_matched;
  int g_use_missing_bit;
  FILE* g_logfile;
  int g_bound_w;
  int g_bound_h;
  float g_skew;
  float g_rotate_degs_before_processing;
  float g_prune_under_area;
  float g_prune_over_area;
  int g_use_ppm;
  int g_update_probs_after_encoding;
  int g_dump_indices;
  int g_insert_spaces;
  int g_rotate_offsets;
  char *g_layout_filename;
  int g_dump_zones;
  char *g_codebook_filename;
  int  g_num1;
  int  g_num2;
  int  g_align_using_centres;

  int  g_match_mode;

  int  g_prune_as_pixels;
} globaltype;

extern globaltype globals;

void  globals_init(void);


#endif





