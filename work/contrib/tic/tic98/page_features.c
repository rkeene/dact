/*
 * Author:   
 *    Stuart Inglis (singlis@internz.co.nz)
 *    (c) 1994-1998
 * 
 * Implementation of lossless textual image compression, using
 * adaptive library encoding and averaging.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "tic98.h"
#include "getopt.h"

#include "boundary.h"
#include "sort_lines.h"
#include "timer.h"

#include "rotate.h"

#include "line.h"
#include "docstrum.h"

#include "codebilevel.h"

static char default_template[]={
    ".ppppp.;"
    "pp222pp;"
    "p22222p;"
    "p22*...;"
};

void
dump_header()
{
  fprintf(stdout,"@arff 3
@relation features
@attribute 'Number' real
@attribute 'Area' real
@attribute 'Avg_area' real
@attribute 'Density' real
@attribute 'Aspect' real
@attribute 'Holes' real
@attribute 'Edges' real
@attribute 'class' string range ( {?} )
@data
");


}

void
calc_features(marklistptr *list)
{
  int len=0;
  float area=0;
  float avg=0,avg_t=0;
  float avg_area=0;
  float density=0,density_t=0;
  float aspect=0,aspect_t=0;
  float holes=0,holes_t=0;
  float edge=0,edge_t=0;
  marklistptr step=NULL;

  len=marklist_length(*list);
  for(step=*list; step; step=step->next){
    area+=step->data.set;
  }

  if(len)
    avg_area=area/len;

  for(step=*list; step; step=step->next){
    density=(step->data.set*1.0/(step->data.w*step->data.h));
    density_t+=density;
  }
  if(len)
    density=density_t/len;

  for(step=*list; step; step=step->next){
    aspect=(step->data.w*1.0/step->data.h);
    aspect_t+=aspect;
  }
  if(len)
    aspect=aspect_t/len;

  for(step=*list; step; step=step->next){
    marktype copy;
    marklistptr list2;
    int i,j;
    int w,h;

    w=step->data.w+2;
    h=step->data.h+2;
    
    marktype_alloc(&copy,w,h);
    marktype_placeat(copy,step->data,1,1);
    
    for(j=0;j<copy.h;j++)
      for(i=0;i<copy.w;i++){
	int p=gpix(copy,i,j);
	ppix(copy,i,j,!p);
      }
    
    list2=extract_all_marks(NULL,copy,1,8);

    holes=marklist_length(list2)-1; if(holes<0) holes=0;

    holes_t+=holes;
    marklist_free(&list2);
    marktype_free(&copy);
  }
  if(len)
    holes=holes_t/len;

  for(step=*list; step; step=step->next){
    int i,j,p;

    edge=0;
    for(j=0;j<step->data.h;j++)
      for(i=0;i<step->data.w;i++){
	p=gpix(step->data,i,j);
	if((gpix(step->data,i-1,j)!=p) || (gpix(step->data,i,j-1)!=p))
	  edge++;
      }
    
    edge_t+=edge*1.0/(step->data.w*step->data.h);

  }
  if(len)
    edge=edge_t/len;
  
  fprintf(stdout,"%d, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f",len,area,avg_area,density,aspect,holes,edge);

}


void
extract_marks_from_zones(marktype image)
{
  FILE *fp;

  fp=fopen(globals.g_layout_filename,"rt");
  if(fp){
    char s[100],tag[100];

    while(fgets(s,100,fp)){
      int x1,y1,x2,y2;
      int i,j;
      marklistptr list=NULL;

      sscanf(s,"%*s %s %d %d %d %d\n",tag,&x1,&y1,&x2,&y2);
      fprintf(stderr,"%s %d %d %d %d\n",tag,x1,y1,x2,y2);

      list=extract_all_marks_bound(NULL,
				   image,
				   globals.g_extraction_nested,
				   globals.g_extraction_connectivity,
				   x1,y1,x2,y2);
      calc_features(&list);
      fprintf(stdout,",%s\n",tag);
      marklist_free(&list);
    }
    fclose(fp);
  } else {
    marklistptr list=NULL;

    list=extract_all_marks_bound(list,
				 image,
				 globals.g_extraction_nested,
				 globals.g_extraction_connectivity,
				 0,0,image.w-1,image.h-1);
    marklist_free(&list);
  }
  
}





void 
usage(void)
{
    fprintf(stderr,
	    "usage: \n"
	    "\ttic98 [-c -d] -i infile -o outfile\n"
	    "\n"
	    "options:\n"
	    "  -c/d\tcompress or decompress an image\n"
	    "  -i n\tSpecify an infile <n>(or series of files...)\n"
	    "  -o n\tSpecify the outfile prefix <n>\n"
	    "  -r n\trotate n degrees before processing\n"
	    "  -s n\tset the known skew to be n degrees\n"
	    "  -w n\tset the component width bound\n"
	    "  -h n\tset the component height bound\n"
	    "  -R m\tset the reading order mode to be m\n"
	    "  -l l\tlog to file <l>\n"
	    "  -Nn\t(N)ested extraction or (n)on-(n)ested extraction\n"
	    "  -48\t4 or 8 connectivity\n"
	    "  -P\tuse PPM to encode indices\n"
	    "  -u\tupdate probabilities after encoding component\n"
	    "  -D d\tset default resolution to d DPI\n"
	    "\n"
	    "information:\n"
	    "\tMultiple pages may be compressed using several\n"
	    "\t-i options. When the file is decompressed the\n"
	    "\t<outfile> name is used as a prefix, and the decompressed\n"
	    "\timages are named <outfile>.001, <outfile>.002, ...\n"
	    "\n"
	    "\tFor example:\n"
	    "\t    tic98 -c -i page1.pbm -i page2.pbm -o compressed\n"
	    "\t    tic98 -d -i compressed -o decompressed\n"
	    "\t    xv decompressed.001 decompressed.002\n"
	    "\n"
	    "version:\n"
	    "\t20 Jan 1998\n"
	    );
    exit(0);
}



int 
main(int argc, char *args[])
{
  int ch;
  extern char *optarg;
  int compress=1;
  int i;
  int num_in=0;
  int num_out=0;
  char **infilename=NULL;
  char **outfilename=NULL;

  globals_init();

  while((ch = getopt(argc, args, "cD:di:o:U:uPnN48l:R:w:h:s:r:p:I:SOf:")) != -1)
    switch(ch)
      {
      case 'f':
	globals.g_layout_filename=(char*)strdup(optarg);
	break;
      case 'r':
	globals.g_rotate_degs_before_processing=DEG2RAD(atof(optarg));break;
      case 's':
	globals.g_skew=DEG2RAD(atof(optarg));break;
      case 'w':
	globals.g_bound_w=atoi(optarg);break;
      case 'h':
	globals.g_bound_h=atoi(optarg);break;
      case 'R':
	if(strcasecmp(optarg,"howard")==0) globals.g_reading_order=EXTRACTION_HOWARD;
	else if(strcasecmp(optarg,"zhang")==0) globals.g_reading_order=EXTRACTION_ZHANG;
	else if(strcasecmp(optarg,"extraction")==0) globals.g_reading_order=EXTRACTION_ORDER;
	else if(strcasecmp(optarg,"profile")==0) globals.g_reading_order=EXTRACTION_PROFILE;
	else if(strcasecmp(optarg,"random")==0) globals.g_reading_order=EXTRACTION_RANDOM;
	else if(strcasecmp(optarg,"docstrum")==0) globals.g_reading_order=EXTRACTION_DOCSTRUM;
	else {
	  fprintf(stderr,"error: unknown Reading order mode\n");
	  exit(1);
	}
	break;
      case 'l':
	if(strcmp(optarg,"-")==0)
	  globals.g_logfile=stderr;
	else
	  globals.g_logfile=fopen(optarg,"wt");
	break;
      case 'n':
	globals.g_extraction_nested=0; break;
      case 'N':
	globals.g_extraction_nested=1; break;
      case '4':
	globals.g_extraction_connectivity=4; break;
      case '8':
	globals.g_extraction_connectivity=8; break;
      case 'p':
	globals.g_prune_under_area=atof(optarg);
	break;
      case 'P':
	globals.g_use_ppm=!globals.g_use_ppm;
	fprintf(stderr,"use_ppm: %d\n",globals.g_use_ppm);
	break;
      case 'D':
	globals.g_default_resolution=atoi(optarg);
	break;
      case 'u':
	globals.g_update_probs_after_encoding=1;
	break;
      case 'U': {
	int i;
	char *ss=(char*)strdup(optarg);
	for(i=0;i<(int)strlen(ss);i++){
	  if(ss[i]==',') ss[i]=' ';
	}
	sscanf(ss,"%d %d %d %d %d %d",
	       &globals.g_context_init,
	       &globals.g_context_increment,
	       &globals.g_context_norm,
	       &globals.g_gamma_init,
	       &globals.g_gamma_increment,
	       &globals.g_gamma_norm
	       );
      }
      break;
      case 'c':
	compress=1;
	break;
      case 'd':
	compress=0;
	break;
      case 'S':
	globals.g_insert_spaces=1;
	break;
      case 'I':
	globals.g_dump_indices=atoi(optarg);
	if((globals.g_dump_indices<0) || (globals.g_dump_indices>2))
	  globals.g_dump_indices=0;
	break;
      case 'i':
	infilename=(char**)realloc(infilename,sizeof(char*)*(num_in+1));
	assert(infilename);
	infilename[num_in++]=(char*)strdup(optarg);
	break;
      case 'o':
	outfilename=(char**)realloc(outfilename,sizeof(char*)*(num_out+1));
	assert(outfilename);
	outfilename[num_out++]=(char*)strdup(optarg);
	break;
      case 'O':
	globals.g_rotate_offsets=1;
	break;
      default:
	usage();
      }

  for(;optind<argc;optind++){
    usage();
  }

  if((infilename==NULL))
    usage();


  /*
   * Arithmetic coders usually read/write to stdin/stdout, but here
   * we'll reassign them to files with "b" mode for compatibility 
   * on non-unix machines 
   */

    for(i=0;i<num_in;i++){
      int w,h;
      marktype image;

      if(marktype_readnamed(infilename[i],&image)!=0)
	return 1;

      extract_marks_from_zones(image);
      marktype_free(&image);
    }

  free(infilename);

  exit(0);
}
