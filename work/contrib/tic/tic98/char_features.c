#include "globals.h"

#include "template_match.h"
#include "marklist.h"


int
main(int argc, char *argv[])
{
  marklistptr list=NULL,step=NULL;
  marklistptr *ar;
  int i,len,r,j,k,k2,k3,k4,k5,k6,k7,rr;
  float r1,r2;
  char *a,*b;
  int iter=0;

  srand(1);
  globals_init();

  if(marklist_readascii(argv[1],&list)==0){
    exit(1);
  }
  len=marklist_length(list);
  fprintf(stderr,"len: %d\n",len);

  ar=malloc(sizeof(marklistptr)*len);

  for(i=0,step=list;step;i++,step=step->next){
    ar[i]=step;
  }

  fprintf(stdout,"@relation chars\n"
	  "@attribute xor real\n"
	  "@attribute wxor real\n"
	  "@attribute wan real\n"
	  "@attribute csis real\n"
	  "@attribute ctm_bits real\n"
	  "@attribute ctm_bpp real\n"
	  "@attribute abs_w real\n"
	  "@attribute abs_h real\n"
	  "@attribute abs_area real\n"
	  "@attribute class {match,differ}\n"
	  "@data\n");

  for(i=0;i<len;i++){
    if((i%100)==0){
      fprintf(stderr,"%d   \r",i);fflush(stderr);
    }
    
    
    a=ar[i]->data.name;
    for(r=0;r<5;r++){ 
      iter=0;
      do{
	j=rand()%len;
	b=ar[j]->data.name;
	iter++;
      } while(((strcmp(a,b)!=0) || (i==j)) && (iter<1000));
      if(ar[i]->data.name[0]=='?')
	iter=10000;
      
      if(iter<1000){
	
	k=XOR_match(&(ar[i]->data),&(ar[j]->data));
	k2=WXOR_match(&(ar[i]->data),&(ar[j]->data));
	k3=WAN_match(&(ar[i]->data),&(ar[j]->data));
	k4=CSIS_match(&(ar[i]->data),&(ar[j]->data));
	CTM_match_proper(&(ar[i]->data),&(ar[j]->data),&r1,&r2);
	fprintf(stdout,"%% %s %s\n",ar[i]->data.name,ar[j]->data.name);
	fprintf(stdout,"%d,%d,%d,%d,%g,%g, %d,%d, %g, match\n",k,k2,k3,k4,r1,r2,
		abs(ar[i]->data.w-ar[j]->data.w),abs(ar[i]->data.h-ar[j]->data.h),fabs(ar[i]->data.set*1.0/(ar[i]->data.w*ar[i]->data.h)-ar[j]->data.set*1.0/(ar[j]->data.w*ar[j]->data.h)));
      }
    }
    
    for(rr=0;rr<20;rr++){
      a=ar[i]->data.name;
      iter=0;
      do{
	j=rand()%len;
	b=ar[j]->data.name;
	iter++;
      } while(((strcmp(a,b)==0) || (i==j)) && (iter<1000));
      
      if(iter<1000){
	
	k=XOR_match(&(ar[i]->data),&(ar[j]->data));
	k2=WXOR_match(&(ar[i]->data),&(ar[j]->data));
	k3=WAN_match(&(ar[i]->data),&(ar[j]->data));
	k4=CSIS_match(&(ar[i]->data),&(ar[j]->data));
	CTM_match_proper(&(ar[i]->data),&(ar[j]->data),&r1,&r2);
	
	fprintf(stdout,"%d,%d,%d,%d,%g,%g, %d,%d, %g, differ\n",k,k2,k3,k4,r1,r2,
		abs(ar[i]->data.w-ar[j]->data.w),abs(ar[i]->data.h-ar[j]->data.h),fabs(ar[i]->data.set*1.0/(ar[i]->data.w*ar[i]->data.h)-ar[j]->data.set*1.0/(ar[j]->data.w*ar[j]->data.h)));
      }
    }
  }
    
    
   
  marklist_free(&list);
  free(ar);
  return 0;

}
