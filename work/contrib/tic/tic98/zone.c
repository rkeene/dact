

#include "line.h"
#include "boundary.h"

void
line_init(linetype *z)
{
  z->num_lines=0;
  z->line=NULL;
}

void 
line_addline(linetype *z, marklistptr list)
{
  int n=z->num_lines;
  marklistptr step,end;

  z->line=(marklistptr*)realloc(z->line,sizeof(marklistptr)*(n+1));
  z->line[n]=NULL;

  for(step=list;step; step=step->next){
    if(z->line[n]==NULL){
      end=marklist_addcopy(&z->line[n],step->data);
    } else {
      end=marklist_addcopy(&end,step->data);
    }
  }
  z->num_lines++;
}

void 
line_addmark(linetype *z, int n, marktype m)
{
  marklist_addcopy(&z->line[n],m);
}

void
line_dump(linetype *z)
{
  int i;
  fprintf(stderr,"z->num_lines: %d\n",z->num_lines);
  for(i=0;i<z->num_lines;i++){
    fprintf(stderr,"line %d: %d\n",i,marklist_length(z->line[i]));
  }
}


void
line_free(linetype *z)
{
  int i;
  for(i=0;i<z->num_lines;i++){
    marklist_free(&z->line[i]);
  }
  free(z->line);
  z->line=NULL;
  z->num_lines=0;
}


void
line_test()
{
  marktype im;
  marklistptr list=NULL;
  linetype z;

  marktype_readnamed("b.pbm",&im);
  list=extract_all_marks(list,im,1,8);

  line_init(&z);
  line_addline(&z,list);
  line_dump(&z);
  marklist_prune_under_area(&list,5,200);
  line_addline(&z,list);
  
  line_dump(&z);
  line_free(&z);

  fprintf(stderr,"line_test() finished\n");
}




