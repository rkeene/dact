/*****************************************************************************
 *
 *   Author:   Stuart Inglis     <singlis@waikato.ac.nz>
 *
 *
 * This file contains functions which are common utilities
 *
 *****************************************************************************/
#ifndef __ROTATE_H
#define __ROTATE_

#include "marklist.h"

void rotate_image(marktype *image,double angle, int keep_orig_size);
void r_hough(marktype image, int threshold);

#endif


