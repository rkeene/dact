#ifndef _CONTEXT_H
#define _CONTEXT_H

#define UPDATE 1
#define COMPRESS 0
#define DECOMPRESS 0

/* the number of choices available for each position in the context. For a
   black and white context CONTEXT_BITS would be 2, but if we wanted to
   represent a value of "MISSING" as well, CONTEXT_BITS would be 3. */

/*#define BINARY*/
#undef BINARY

#ifdef BINARY
#define CONTEXT_BITS 2
#else
#define CONTEXT_BITS 3
#endif


/* the number of bits for the standard JBIG template */
#define JBIG_SIZE 10

#define MAX_CONTEXT_SIZE 30

extern int JBIG_CLAIR_1;
extern int JBIG_CLAIR_2;
extern int JBIG_CLAIR_SIZE;

typedef struct {
  int w,b;
} probtype;

typedef struct {
  int size;
  int max_value;
  probtype *p;
} contexttype;                                                                 

typedef struct {
  int x,y;
} coordinate;


void  context_init(contexttype *c, int size);
void  context_free(contexttype *c);
void  context_update(contexttype *c, int mask, int w);
void  context_encode(contexttype *c, int mask, int p);
float context_prob(contexttype *c, int mask, int p);
int   context_decode(contexttype *c, int mask);
void  context_clear(contexttype *c);

int   jbig_mask(marktype *m,int x, int y);
int   jbig_mask_clair(marktype *m,marktype *clair,int xo,int yo,int x, int y);
void  compress_mark(contexttype *con, marktype *m, int st);
void  compress_mark_clair(contexttype *con, marktype *m, marktype *cl,
			 int xco, int yco, int st) ;
void  decompress_mark(contexttype *con, marktype *m, int w,int h);
void  decompress_mark_clair(contexttype *con, marktype *m, marktype *cl,
			   int w,int h,int xco, int yco);



#endif

