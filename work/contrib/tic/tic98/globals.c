#include "globals.h"

globaltype globals;

void
globals_init(void)
{
  globals.g_reading_order=EXTRACTION_DOCSTRUM;
  globals.g_reading_parameter=1;        /* mm */
  globals.g_default_resolution=300;     /* 300 */
  globals.g_extraction_nested=1;        /* nested */
  globals.g_extraction_connectivity=8;  /* 8 */
  globals.g_gamma_init=1;
  globals.g_gamma_increment=1;
  globals.g_gamma_norm=256;  
  globals.g_max_classes=UNLIMITED;
  globals.g_max_examples=UNLIMITED;
  globals.g_match_threshold=400;           /* 21 % */
  globals.g_context_init=1;
  globals.g_context_increment=1;
  globals.g_context_norm=256;
  globals.g_marks_on_page=0;
  globals.g_num_matched=0;
  globals.g_use_missing_bit=0;          /* 0 */
  globals.g_logfile=NULL;
  globals.g_bound_w=0;
  globals.g_bound_h=0;
  globals.g_skew=0;
  globals.g_rotate_degs_before_processing=0;
  globals.g_prune_under_area=0;
  globals.g_prune_over_area=0;
  globals.g_use_ppm=0;
  globals.g_update_probs_after_encoding=1; /* 1*/
  globals.g_dump_indices=0;
  globals.g_insert_spaces=0;
  globals.g_rotate_offsets=0;
  globals.g_layout_filename=NULL;
  globals.g_dump_zones=0;
  globals.g_codebook_filename=NULL;
  globals.g_num1=4;
  globals.g_num2=9;
  globals.g_align_using_centres=0;

  globals.g_match_mode=MATCH_B_AVG;

  globals.g_prune_as_pixels=0;
}


