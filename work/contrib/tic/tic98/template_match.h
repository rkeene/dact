#ifndef __MATCH_H
#define __MATCH_H

#include <math.h>
#include "marklist.h"


#define NOT_SCREENED(b1,b2) ((abs((b1).w-(b2).w)<=2) && (abs((b1).h-(b2).h)<=2))

int XOR_match(marktype *b1, marktype *b2);

int WXOR_match(marktype *b1, marktype *b2);
int WAN_match(marktype *b1, marktype *b2);
int PMS_match(marktype b1, marktype b2);
int CSIS_match(marktype *b1, marktype *b2);
int CTM_match(marktype *b1, marktype *b2);
int ALL_match(marktype *b1, marktype *b2);

void CTM_match_proper(marktype *b1, marktype *b2, float *bits, float *bpp);


/*void CTM_match(marktype *b1, marktype *b2, float *bits, float *bpp);
  int CTM_match_c5(marktype *b1, marktype *b2);*/

#endif
