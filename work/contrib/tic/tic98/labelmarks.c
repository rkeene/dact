#include <stdio.h>
#include <string.h>
#include "globals.h"

#include "template_match.h"
#include "marklist.h"


int
main(int argc, char *argv[])
{
  marklistptr list=NULL,step=NULL;
  int i,len,r,j,k,k2,k3,k4,k5,k6,k7;
  float r1,r2;
  char *a,*b;
  char label[1000],temp[1000];
  int q,pos;

  srand(1);

  if(marklist_readascii(argv[1],&list)==0){
    exit(1);
  }
  len=marklist_length(list);
  fprintf(stderr,"len: %d\n",len);

  step=list;
  pos=0;
  while(1){
    marktype_writeascii(stderr,step->data);
    fprintf(stderr,"%d/%d label: ",pos,len-1);
    fflush(stderr);
    strcpy(temp,"");
    if(fgets(temp,100,stdin)==NULL)
      break;
    sscanf(temp,"%s",label);
    if(strlen(label)>0){
      if(step->data.name){
	free(step->data.name);
      }
      step->data.name=strdup(label);
    } 
    fprintf(stderr,"label: %s\n",step->data.name?step->data.name:"?");
    if(step && step->next){
      step=step->next;
      pos++;
    }
  } ;

  for(step=list;step;step=step->next){
    marktype_writeascii(stdout,step->data);
  }
   
  marklist_free(&list);
  return 0;

}
