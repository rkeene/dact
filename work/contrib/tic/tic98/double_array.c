#include <stdio.h>
#include "utils.h"
#include "double_array.h"


/*
** da_init() initialises a real valued array between [low,high] inclusive
*/

void da_init(doublearraytype *da,double low, double high, double binsize)
{
    int NUM;

    da->low=low;
    da->high=high;
    da->binsize=binsize;

    if(high<=low) {
	fprintf(stderr,"eek! high must be > low!\n");
	exit(1);
    }

    if((binsize<=0)){
	fprintf(stderr,"eek! binsize > 0 !\n");
	exit(1);
    }

    NUM=ROUND((high-low)/binsize);
    if(NUM==0){
	fprintf(stderr,"low %g, high %g, binsize %g, NUM=%d, exiting...\n",low,high,binsize,NUM);
	exit(1);
    }
    CALLOC(da->ary,NUM,double);
    da->size=NUM;
}


void da_free(doublearraytype *da)
{
    if(da->size>0){
	FREE(da->ary);
	da->size=0;
    } else {
	fprintf(stderr,"already empty!\n");
    }
}

void da_copy(doublearraytype *copy, doublearraytype *orig)
{
    int i;

    copy->low=orig->low;
    copy->high=orig->high;
    copy->binsize=orig->binsize;
    copy->size=orig->size;

    for(i=0;i<copy->size;i++)
	copy->ary[i]=orig->ary[i];
}


int da_idx(doublearraytype *da,double val)
{
    int NUM;
    
    if(val==da->high)
	NUM=da->size;
    else
	NUM=ROUND((val-da->low)/da->binsize);
    if((NUM<0) || (NUM>da->size)) {
	fprintf(stderr,"eeek out of range. %g in [%g,%g] [%d %d]\n",val,da->low,da->high,NUM,da->size);
    }
    return NUM;
}



void da_write(doublearraytype *da, const char filename[])
{
    FILE *fp;
    int i;

    if(!(fp=fopen(filename,"wb"))){
	fprintf(stderr,"can't create file: %s\n",filename);
	exit(1);
    }
    
    for(i=0;i<da->size;i++)
	fprintf(fp,"%g %g\n",(i*da->binsize)+da->low,da->ary[i]);

    fclose(fp);
}

double da_val(doublearraytype *da,int p)
{
    if((p<0) || (p>da->size)){
	fprintf(stderr,"accessing %d, out of the range [%d,%d]\n",p,0,da->size);
	exit(1);
    }
	
    return (p*da->binsize)+da->low;
}
    
    



