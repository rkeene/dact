#include "boundary.h"

#include "template_match.h"
#include "marklist.h"


int
main(int argc, char *argv[])
{
  marklistptr list=NULL,step=NULL;
  marktype image;
  int i,len,r,j,k,k2,k3,k4,k5,k6,k7;
  float r1,r2;
  char *a,*b;

  srand(1);

  marktype_readnamed(argv[1],&image);

  list=extract_all_marks(list,image,1,8);

  for(step=list;step;step=step->next){
    marktype_writeascii(stdout,step->data);
  }
   
  marklist_free(&list);
  marktype_free(&image);

  return 0;

}
