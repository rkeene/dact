#ifndef _CODEBOOK_H
#define _CODEBOOK_H

#include "marklist.h"

extern int SCALE;


#define NO_UPDATE  0
#define UPDATETIME 1

/* XOR 17
   WXOR 58
   WAN 76
   CSIS 400
   CTM 9999
   ALL 9999
*/
#define TEMP_FUNC        CSIS_match
#define TEMP_FUNC_STRING "CSIS"


typedef struct {
  int num;
  marktype avg;
  int avg_uptodate;
  marktype *set;
  int max_examples;
  int time;
} equiv_classes;

typedef struct {
  int num_eqv;
  equiv_classes *eqv;
  int max_classes;
  int max_examples;
  int time;
} codebook;


void      codebook_init(codebook *c, int max_classes, int max_examples);
void      codebook_free(codebook *c);
void      codebook_updatetime(codebook *c);

int       codebook_add_equiv(codebook *c);
int       codebook_add_equiv2(codebook *c);

int       codebook_add_mark(codebook *c,int eq,marktype *mark);
int       codebook_add_mark2(codebook *c,int eq,marktype *mark);

int       codebook_match(codebook *c,marktype *mark);
void      codebook_dump(codebook *c, int all);
void      codebook_dump_pbm(codebook *c, char *fn);
void      equiv_classes_average_mark(equiv_classes *e, marktype *mark);

int       codebook_match_against_average(codebook *c,marktype *mark);
int       codebook_match_against_average2(codebook *c,marktype *mark);

marktype *codebook_get_avg_mark(codebook *c, int eq, int updatetimeaccess);
marktype *codebook_get_avg_mark2(codebook *c, int eq, int updatetimeaccess);

void      codebook_configure(codebook *c, int max_classes, int max_examples);



#endif
