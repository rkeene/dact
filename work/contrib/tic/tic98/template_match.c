/*
 * Module: template_match
 *
 *
 * Author:
 *   Stuart Inglis (singlis@internz.co.nz)
 *   (c) 1998
 */

#include "template_match.h"
#include "marklist.h"
#include "pbmtools.h"
#include "utils.h"

#include "globals.h"

#include <math.h>




int XOR_match(marktype *b1, marktype *b2)
{
  static int r,c;
  static int offx,offy;
  static int p1,p2;
  static int count;
  static int tr,tc;
  static int totalbox,totalbox2;
  static int x1m,y1m,x2M,y2M;
    
  offx=b1->xcen-b2->xcen;
  offy=b1->ycen-b2->ycen;

  x1m=offx;
  y1m=offx;
  x2M=b2->w+offx;
  y2M=b2->h+offy;

  x1m=MIN(x1m,0);
  y1m=MIN(y1m,0);
  x2M=MAX(x2M,b1->w);
  y2M=MAX(y2M,b1->h);

  /*
  x1m=0;  
  y1m=0; 
  x2M=b1->w; 
  y2M=b1->h;
  */
    
  count=0;
    
  for(r=y1m;r<y2M;r++)
    for(c=x1m;c<x2M;c++){
      p1=gpix((*b1),c,r);
      tc=c-offx;
      tr=r-offy;
      p2=gpix((*b2),tc,tr);
      if(p1!=p2) {
	count++;
      }
    }
  totalbox=(x2M-x1m)*(y2M-y1m);
    
  return 100*count/totalbox;
}




/* error mask goes from 0..8, single pixel errors are worth nothing */
int WXOR_match(marktype *b1, marktype *b2)
{
  int r,c,i,j;
  int minr,minc;
  int maxr,maxc;
  int offx,offy;
  int p1,p2;
  int count=0;
  marktype d;
  int totalbox;
  
  offx=b1->xcen-b2->xcen;
  offy=b1->ycen-b2->ycen;
  
  minc=MIN(0,offx);
  minr=MIN(0,offy);
  
  maxc=MAX(b1->w,b2->w+offx);
  maxr=MAX(b1->h,b2->h+offy);
  
  d.w=maxc-minc+1;
  d.h=maxr-minr+1;
  marktype_alloc(&d,d.w,d.h);

  
  for(r=minr;r<maxr;r++)
    for(c=minc;c<maxc;c++){
      p1=gpix((*b1),c,r);
      p2=gpix((*b2),c-offx,r-offy);
      pbm_putpixel(d.bitmap,c-minc,r-minr,p1!=p2);
    }
  for(r=minr;r<maxr;r++)
    for(c=minc;c<maxc;c++)if(pbm_getpixel(d.bitmap,c-minc,r-minr))
      for(i=-1;i<=1;i++)
	for(j=-1;j<=1;j++)if((r+j>=0) && (c+i>=0) && (r+j<d.h) && (c+i<d.w) &&((i!=0)||(j!=0))&&(pbm_getpixel(d.bitmap,c+i-minc,r+j-minr)))
	  count++;
  marktype_free(&d);

  totalbox=(maxc-minc)*(maxr-minr);

  return 100*count/totalbox;
}




/* define an array for most matches, if it is a massive mark, then WXOR is called */
#define S_SIZE 500

static unsigned char scratch[S_SIZE][S_SIZE],sc2[S_SIZE][S_SIZE];




/* Weighted AND-NOT */

int WAN_match(marktype *b1, marktype *b2)
{
    int r,c,i,j;
    int minr,minc;
    int maxr,maxc;
    int offx,offy;
    int p1,p2;
    int count=0;
    int totalbox;
    
    offx=b1->xcen-b2->xcen;
    offy=b1->ycen-b2->ycen;
    
    minc=MIN(0,offx);
    minr=MIN(0,offy);
    
    maxc=MAX(b1->w,b2->w+offx);
    maxr=MAX(b1->h,b2->h+offy);
    
    if(((maxc-minc)>=S_SIZE)||((maxr-minr)>=S_SIZE))  
      return WXOR_match(b1,b2);
    
    for(r=minr;r<maxr;r++)
      for(c=minc;c<maxc;c++){
	p1=gpix((*b1),c,r);
	p2=gpix((*b2),c-offx,r-offy);
	if((p1==0)&&(p2==1))   /* p1 not set, p2 = set */
	  scratch[r-minr][c-minc]=1;
	else 
	  scratch[r-minr][c-minc]=0;	  

	if((p1==1)&&(p2==0))  /* p1 set, p2 not set */
	  sc2[r-minr][c-minc]=1;
	else
	  sc2[r-minr][c-minc]=0;
	  
      }
    
    count=0;
    for(r=minr;r<maxr;r++)
      for(c=minc;c<maxc;c++)if(scratch[r-minr][c-minc])
	for(i=-1;i<=1;i++)
	  for(j=-1;j<=1;j++)if((r+j>=minr) && (c+i>=minc) && (r+j<maxr) && (c+i<maxc) &&((i!=0)||(j!=0))&&scratch[r-minr+j][c-minc+i])
	    count++;
    
    for(r=minr;r<maxr;r++)
      for(c=minc;c<maxc;c++)if(sc2[r-minr][c-minc])
	for(i=-1;i<=1;i++)
	  for(j=-1;j<=1;j++)if((r+j>=minr) && (c+i>=minc) && (r+j<maxr) && (c+i<maxc) &&((i!=0)||(j!=0))&&sc2[r-minr+j][c-minc+i])
	    count++;
    
    totalbox=(maxc-minc)*(maxr-minr);
    
    return 100*count/totalbox;
}

/* oc 8 must be in a clock wise order, with oc[0] a corner */
struct {int x,y;} oc[8]={{-1,-1},{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0}};



static void xormatchscratch(marktype *b1, marktype *b2, int minr, int maxr, int minc, int maxc, int offx, int offy)
{
  int r,c,rmm,roo,coo;
  int p1,p2;
  int w1=b1->w,h1=b1->h,w2=b2->w,h2=b2->h;

  for(r=minr; r<maxr;r++){
    rmm=r-minr; roo=r-offy;
    p1=0;
    for(c=minc;c<0;c++){
      coo=c-offx;
      scratch[rmm][c-minc]=sc2[rmm][c-minc]=0;
      if((coo>=0)&&(roo>=0)&&(coo<w2)&&(roo<h2))p2=pbm_getpixel(b2->bitmap,coo,roo);
      else p2=0; /* background */
      scratch[rmm][c-minc]=p1!=p2;
    }
    for(c=0;c<maxc;c++){
      coo=c-offx;
      scratch[rmm][c-minc]=sc2[rmm][c-minc]=0;
      if((r>=0)&&(c<w1)&&(r<h1))p1=pbm_getpixel(b1->bitmap,c,r);
      else p1=0; /* background */
      if((coo>=0)&&(roo>=0)&&(coo<w2)&&(roo<h2))p2=pbm_getpixel(b2->bitmap,coo,roo);
      else p2=0; /* background */
      scratch[rmm][c-minc]=p1!=p2;
    }
  }
}



int CSIS_match(marktype *b1, marktype *b2)
{
    int r,c,i1;
    int minr,minc;
    int maxr,maxc;
    int offx,offy,fepcount;
    int totscratch=0;

    
    offx=b1->xcen-b2->xcen;
    offy=b1->ycen-b2->ycen;
    
    minc=MIN(0,offx);
    minr=MIN(0,offy);
    
    maxc=MAX(b1->w,b2->w+offx);
    maxr=MAX(b1->h,b2->h+offy);
    
    if(((maxc-minc)>=S_SIZE)||((maxr-minr)>=S_SIZE)) 
      return WXOR_match(b1,b2);

    /* set the scratch array to 1 if there is an XOR match */

    xormatchscratch(b1,b2,minr,maxr,minc,maxc,offx,offy);
    /* We how have the XOR map. Scratch contains the number of neighbours, 
       1=1 neighbour set...etc */

    /* CSIS rule 1, if FEP (four error pels) then exit */

    

    fepcount=0;

    for(r=minr;r<maxr-1;r++)
	for(c=minc;c<maxc-1;c++) 
	    if( (scratch[r-minr][c-minc])&& (scratch[r-minr+1][c-minc])&&
	       (scratch[r-minr][c-minc+1])&& (scratch[r-minr+1][c-minc+1])) {
		fepcount++;
		if(fepcount==1) {
		    /* fprintf(stderr,"position (%d %d)\n",c-minc,r-minr);*/ 
		    return 10001;   
		}
	    }
    
    /* foreach set pixel, update scratch with the number of pels around in five dir's. 
       FPN (five-pel neighbour hood) */
    for(r=minr;r<maxr;r++)
      for(c=minc;c<maxc;c++) if(scratch[r-minr][c-minc]){
	sc2[r-minr][c-minc]=1; /* itself */
	for(i1=1;i1<8;i1+=3)
	  if(((r+oc[i1].y)>=minr)&&((c+oc[i1].x)>=minc)&&((r+oc[i1].y)<maxr)&&((c+oc[i1].x)<maxc))
	    if(scratch[r+oc[i1].y-minr][c+oc[i1].x-minc]) sc2[r-minr][c-minc]++;
      }
    /* now scratch has the value of the FPN - 1=single error, 5=4 neighbouring errors etc... */
    
    /* set scratch and sc2 to be the same */
    for(r=minr;r<maxr;r++)
      for(c=minc;c<maxc;c++)
	totscratch+=scratch[r-minr][c-minc]=sc2[r-minr][c-minc];

    for(r=minr;r<maxr;r++)
      for(c=minc;c<maxc;c++){
	/* CSIS rule 2i: an error pel has 2 or more neighbours---this num is the same as PMS */
	if(scratch[r-minr][c-minc]>=3)     
	  if((r-minr>=1) && (c-minc>=1) && (r-minr<(maxr-1)) && (c-minc<(maxc-1))){
/**/	    int i2,numsep=0;
	    /* find how many neighbours a set pixel has, only check the 4 edges-FPN */
	    for(i2=1;i2<8;i2+=2)
	      if(scratch[r+oc[i2].y-minr][c+oc[i2].x-minc]) { /* if a scratch pixel is set */
		  if((scratch[r+oc[(i2+2)%8].y-minr][c+oc[(i2+2)%8].x-minc]==0)&&
		     (scratch[r+oc[(i2+6)%8].y-minr][c+oc[(i2+6)%8].x-minc]==0)
		     ) numsep++;
		}
	    
	    if((b1->w>=12)||(b2->w>=12)||(b1->h>=12)||(b2->h>=12)){
	      
	      /* PMS rule 2ii: at least two of its neigh. error p's aren't connected */
	      if(numsep>=2){  
		int i,j,p,p2,same;
		
		/* PMS rule 2(iii), look for regions in the original images of 3x3 the same */
		if((c>=0)&&(r>=0)&&(c<b1->w)&&(r<b1->h)){
		  p=pbm_getpixel(b1->bitmap,c,r);
		  same=0;
		  for(i=-1;i<=1;i++)
		    for(j=-1;j<=1;j++){
		      if((c+i>=0)&&(r+j>=0)&&(c+i<b1->w)&&(r+j<b1->h))
			p2=pbm_getpixel(b1->bitmap,c+i,r+j);
		      else
			p2=0;
		      if(p2==p) same++;
		    } /* i,j loop */
		  /* PMS rule 2(iii) */
		  if(same==9) {return 10002;}
		} /* (*b1) in range */
		
		if((c-offx>=0) && (r-offy>=0) &&(c-offx<b2->w) && (r-offy<b2->h)){
		  p=pbm_getpixel(b2->bitmap,c-offx,r-offy);
		  same=0;
		  for(i=-1;i<=1;i++)
		    for(j=-1;j<=1;j++){
		      if((c+i-offx>=0)&&(r+j-offy>=0)&&(c+i-offx<b2->w)&&(r+j-offy<b2->h))
			p2=pbm_getpixel(b2->bitmap,c+i-offx,r+j-offy);
		      else 
			p2=0;
		      if(p2==p) same++;
		    } /* i,j loop */
		  /* PMS rule 2(iii) */
		  if(same==9) {return 10003;}
		}
	      }
	    } 
	    else
	      { /* do the CSIS rule */
		/* CSIS rule 2ii: at least two of its neigh. error p's aren't connected */
		if(numsep>=2){  
		  int i,p,p2,same;
		  
		  /* PMS rule 2(iii), look for regions in the original images of 3x3 the same */
		  if((c>=0)&&(r>=0)&&(c<b1->w)&&(r<b1->h)){
		    p=pbm_getpixel(b1->bitmap,c,r);
		    same=1;
		    for(i=1;i<8;i+=2)
		      if((c+oc[i].x>=0)&&(r+oc[i].y>=0)&&(c+oc[i].x<b1->w)&&(r+oc[i].y<b1->h))
			p2=pbm_getpixel(b1->bitmap,c+oc[i].x,r+oc[i].y);
		      else
			p2=0;
		    if(p2==p) same++;
		    
		    /* CSIS rule 2(iii) */
		    if(same==5) {return 10002;}
		  }
		  
		  if((c-offx>=0) && (r-offy>=0) &&(c-offx<b2->w) && (r-offy<b2->h)){
		    p=pbm_getpixel(b2->bitmap,c-offx,r-offy);
		    same=1;
		    for(i=1;i<8;i+=2)
		      if((c+oc[i].x-offx>=0)&&(r+oc[i].y-offy>=0)&&(c+oc[i].x-offx<b2->w)&&(r+oc[i].y-offy<b2->h))
			p2=pbm_getpixel(b2->bitmap,c+oc[i].x-offx,r+oc[i].y-offy);
		      else 
			p2=0;
		    if(p2==p) same++;
		    
		    /* CSIS rule 2(iii) */
		    if(same==5) {return 10003;}
		  }
		}
	      } 
	  }
      }
    
    return totscratch;
}





#include "context.h"

typedef struct {
  int x,y;
} coord;

coord ctm[5]={{0,0},{-1,0},{0,1},{1,0},{0,-1}};

int 
ctm_mask(marktype *m,int x, int y)
{
    int mask=0;
    int i,p;
    int ti,tj;

    assert(m);

    /*-1 because of the last 'allin' attribute */
    for(i=0;i<5;i++){
	ti=x+ctm[i].x;
	tj=y+ctm[i].y;
	p=gpix((*m),ti,tj);
	mask=2*mask | p;
    }
    return mask;
}


void
CTM_match1(marktype *b1, marktype *b2, float *bits, float *bpp)
{
  static int r,c;
  static int offx,offy;
  static int p1,p2;
  static int count;
  static int tr,tc;
  static int totalbox,totalbox2;
  static int x1m,y1m,x2M,y2M;
  int mask;
  static contexttype ct;
  static int first=1;

  if(first){
    context_init(&ct,5);
    first=0;
  } else {
    context_clear(&ct);
  }
    
  offx=b1->xcen-b2->xcen;
  offy=b1->ycen-b2->ycen;

  x1m=offx;
  y1m=offx;
  x2M=b2->w+offx;
  y2M=b2->h+offy;

  x1m=MIN(x1m,0);
  y1m=MIN(y1m,0);
  x2M=MAX(x2M,b1->w);
  y2M=MAX(y2M,b1->h);

  globals.g_context_increment=4;
    
  count=0;
    
  for(r=y1m;r<y2M;r++)
    for(c=x1m;c<x2M;c++){
      mask=ctm_mask(b1,c,r);
      tc=c-offx;
      tr=r-offy;
      p2=gpix((*b2),tc,tr);
      context_update(&ct,mask,p2);
      /*
      p2=gpix((*b2),tc-1,tr);
      context_update(&ct,mask,p2);
      p2=gpix((*b2),tc,tr-1);
      context_update(&ct,mask,p2);
      */
    }
  
  *bits=0;
  for(r=y1m;r<y2M;r++)
    for(c=x1m;c<x2M;c++){
      mask=ctm_mask(b1,c,r);
      tc=c-offx;
      tr=r-offy;
      p2=gpix((*b2),tc,tr);
      *bits=*bits-LOG2(context_prob(&ct,mask,p2));
    }
  
  totalbox=(x2M-x1m)*(y2M-y1m);

  *bpp=*bits/totalbox;

}

void
CTM_match_proper(marktype *b1, marktype *b2, float *bits, float *bpp)
{
  float bits1,bpp1;
  float bits2,bpp2;

  CTM_match1(b1,b2,&bits1,&bpp1);
  CTM_match1(b2,b1,&bits2,&bpp2);

  if(bits1>bits2)
    *bits=bits1;
  else
    *bits=bits2;

  if(bpp1>bpp2)
    *bpp=bpp1;
  else
    *bpp=bpp2;


}

int
CTM_match(marktype *b1, marktype *b2)
{
  float ctm_bits,ctm_bpp;
  int xor,wan,wxor,csis;

  CTM_match_proper(b1,b2,&ctm_bits,&ctm_bpp);
  wxor=WXOR_match(b1,b2);
  xor=XOR_match(b1,b2);
  wan=WAN_match(b1,b2);
  /*csis=CSIS_match(b1,b2);*/
  

  /* the following gives 66808 for A006 */
  /*
    if(ctm_bpp <= 0.656863){
    if(ctm_bits <= 1027.96) return (int)(ctm_bits);
    else return 10000;
    } else {
    if(ctm_bits <= 68.7502) return (int)(ctm_bits);
    else return 10000;
    }
    */

 
  /* the following gives 68139 for A006 */
 /*
    if(ctm_bpp > 0.720843) return 10000;
    else {
    if(ctm_bits <= 352.922) return (int)(ctm_bits);
    else {
    if(ctm_bits > 1072.59) return  100;
    else {
    if(ctm_bpp <= 0.601148) return (int)(ctm_bits);
    else return 10000;
    }
    }
    }
    */

  /* the following gives 65774 for A006 */
  /*
    if(ctm_bpp > 0.568514) return 10000;
    else {
    if(ctm_bits <= 929.215) return (int)(ctm_bits);
    else return 10000;
    }
    */

  /* the following gives 65315/(65296 using ctm_bits) for A006 */

 if(ctm_bpp > 0.598722) return 10000;
  else {
    if(ctm_bpp <= 0.440472) return (int)(ctm_bits); 
    else {
      if(ctm_bits <= 325.267){
	if(ctm_bpp <= 0.524121) return (int)(ctm_bits);
	else {
	  if(ctm_bits > 146.298) return 10000;
	  else {
	    if(ctm_bpp <= 0.573467) return (int)(ctm_bits);
	    else return 10000 ; 
	  }
	}
      } else {
	if(ctm_bpp > 0.546217) return 10000;
	else {
	  if(ctm_bits <= 568.337) return 10000;
	  else {
	    if(ctm_bits <= 757.753) return (int)(ctm_bits); 
	    else return 10000;
	  }
	}
      }
    }
  }


  /*hand classified labels */
  /*
  if(ctm_bpp > 0.683592) return 10000; 
  else {
    if(ctm_bpp <= 0.543907){
      if(ctm_bits > 81.8062) return (int)(ctm_bits);  
      else {
	if(ctm_bpp <= 0.42415) return (int)(ctm_bits);
	else return 10000;
      }
    } else {
      if(ctm_bits > 250.39){
	if(ctm_bits <= 660.765) return (int)(ctm_bits);  
	else return 10000;
      } else {
	if(ctm_bpp > 0.604062) return 10000;  
	else {
	  if(ctm_bits <= 181.421) return 10000; 
	  else return (int)(ctm_bits);
	}
      }
    }
  }
  */
                                                         
  /** all **/

  
  /*

  if (wan <= 92 ){
    if(wan <= 58){
      if(xor > 11){
	if(wan <= 49.5) return   (int)(ctm_bits);
	else if(wan > 49.5 ){
	  if(xor <= 13.5) return  10000;
	  else if(xor > 13.5) return  (int)(ctm_bits) ;
	}
      } else if(xor <= 11){
	if(wan > 29.5){
	  if(wan > 35.5) return  10000;
	  else {
	    if(xor <= 9.5) return  10000 ;
	    else return (int)(ctm_bits) ;
	  }
	} else {
	  if(ctm_bpp <= 0.300983) return  (int)(ctm_bits) ;
	  else {
	    if(xor > 8.5) return   (int)(ctm_bits) ;
	    else {
	      if( wan <= 20.5) return    (int)(ctm_bits);
	      else return 10000;
	    }
	  }
	}
      }
    } else {
      if(ctm_bpp > 0.695644) return  10000 ;
      else {
	if(xor <= 15) return  10000 ;
	else {
	  if(wan <= 75 ){
	    if(xor > 19) return   (int)(ctm_bits) ;
	    else {
	      if(xor <= 19){
		if(ctm_bpp <= 0.588463) return   (int)(ctm_bits) ;
		else return 10000;
	      }
	    }
	  } else {
	    if(xor <= 20 ){
	      if(xor <= 18) return   10000 ;
	      else {
		if(wan > 84) return  10000 ;
		else {
		  if(ctm_bits <= 422.966) return  10000 ;
		  else return (int)(ctm_bits) ;
		}
	      }
	    } else {
	      if(xor > 23) return   (int)(ctm_bits) ;
	      else {
		if(wan <= 82) return   (int)(ctm_bits) ;
		else {
		  if(ctm_bits <= 298.337) return  10000;
		  else return (int)(ctm_bits) ;
		}
	      }
	    }
	  }
	}
      }
    }
  } else {
    if(ctm_bpp > 0.746178) return  10000 ;
    else {
      if(wan > 123){
	if(ctm_bits > 158.702) return  10000 ;
	else {
	  if(xor <= 30) return  10000;
	  else {
	    if(wan <= 157) return   (int)(ctm_bits);
	    else return 10000;
	  }
	}
      } else {
	if(xor <= 23) return  10000 ;
	else {
	  if(wan > 107) return  10000 ;
	  else {
	    if(ctm_bpp <= 0.617752) return   (int)(ctm_bits) ;
	    else {
	      if(wxor <= 91) return  10000 ;
	      else {
		if(wan > 103) return  10000;
		else {
		  if(ctm_bits <= 314.157) return  10000;
		  else return (int)(ctm_bits) ;
		}
	      }
	    }
	  }
	}
      }
    }
    
  }
  
  */
  
  abort();
  return -1;
  
}



int
ALL_match(marktype *b1, marktype *b2)
{
  float ctm_bits,ctm_bpp;
  int xor,wan,wxor,csis;

  CTM_match_proper(b1,b2,&ctm_bits,&ctm_bpp);
  wxor=WXOR_match(b1,b2);
  xor=XOR_match(b1,b2);
  wan=WAN_match(b1,b2);
  /*csis=CSIS_match(b1,b2);*/

  /** all **/


  if (wan <= 92 ){
    if(wan <= 58){
      if(xor > 11){
	if(wan <= 49.5) return   (int)(ctm_bits);
	else if(wan > 49.5 ){
	  if(xor <= 13.5) return  10000;
	  else if(xor > 13.5) return  (int)(ctm_bits) ;
	}
      } else if(xor <= 11){
	if(wan > 29.5){
	  if(wan > 35.5) return  10000;
	  else {
	    if(xor <= 9.5) return  10000 ;
	    else return (int)(ctm_bits) ;
	  }
	} else {
	  if(ctm_bpp <= 0.300983) return  (int)(ctm_bits) ;
	  else {
	    if(xor > 8.5) return   (int)(ctm_bits) ;
	    else {
	      if( wan <= 20.5) return    (int)(ctm_bits);
	      else return 10000;
	    }
	  }
	}
      }
    } else {
      if(ctm_bpp > 0.695644) return  10000 ;
      else {
	if(xor <= 15) return  10000 ;
	else {
	  if(wan <= 75 ){
	    if(xor > 19) return   (int)(ctm_bits) ;
	    else {
	      if(xor <= 19){
		if(ctm_bpp <= 0.588463) return   (int)(ctm_bits) ;
		else return 10000;
	      }
	    }
	  } else {
	    if(xor <= 20 ){
	      if(xor <= 18) return   10000 ;
	      else {
		if(wan > 84) return  10000 ;
		else {
		  if(ctm_bits <= 422.966) return  10000 ;
		  else return (int)(ctm_bits) ;
		}
	      }
	    } else {
	      if(xor > 23) return   (int)(ctm_bits) ;
	      else {
		if(wan <= 82) return   (int)(ctm_bits) ;
		else {
		  if(ctm_bits <= 298.337) return  10000;
		  else return (int)(ctm_bits) ;
		}
	      }
	    }
	  }
	}
      }
    }
  } else {
    if(ctm_bpp > 0.746178) return  10000 ;
    else {
      if(wan > 123){
	if(ctm_bits > 158.702) return  10000 ;
	else {
	  if(xor <= 30) return  10000;
	  else {
	    if(wan <= 157) return   (int)(ctm_bits);
	    else return 10000;
	  }
	}
      } else {
	if(xor <= 23) return  10000 ;
	else {
	  if(wan > 107) return  10000 ;
	  else {
	    if(ctm_bpp <= 0.617752) return   (int)(ctm_bits) ;
	    else {
	      if(wxor <= 91) return  10000 ;
	      else {
		if(wan > 103) return  10000;
		else {
		  if(ctm_bits <= 314.157) return  10000;
		  else return (int)(ctm_bits) ;
		}
	      }
	    }
	  }
	}
      }
    }
    
  }
  
  abort();
  return -1;
  
}
