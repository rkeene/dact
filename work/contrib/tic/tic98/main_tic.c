/*
 * Author:   
 *    Stuart Inglis (singlis@internz.co.nz)
 *    (c) 1994-1999
 * 
 * Implementation of lossless textual image compression, using
 * adaptive library encoding and averaging.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arithcode.h"
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

marklistptr 
extract_marks_from_zones(marktype image)
{
  marklistptr list=NULL;
  FILE *fp;
  marktype copy;

  copy=marktype_copy(image);

  fp=fopen(globals.g_layout_filename,"rt");
  if(fp && (globals.g_dump_zones==0)){
    char s[100],tag[100];


    while(fgets(s,100,fp)){
      int x1,y1,x2,y2;
      sscanf(s,"%*s %s %d %d %d %d\n",tag,&x1,&y1,&x2,&y2);
      if((strncmp(tag,"text",4)==0)){
	int i,j;

	list=extract_all_marks_bound(list,
				     copy,
				     globals.g_extraction_nested,
				     globals.g_extraction_connectivity,
				     x1,y1,x2,y2);
      }
    }
    fclose(fp);
  } else {
    list=extract_all_marks_bound(list,
				 copy,
				 globals.g_extraction_nested,
				 globals.g_extraction_connectivity,
				 0,0,image.w-1,image.h-1);
  }

  marktype_free(&copy);
  
  {
    int i,j;
    marktype recon;
    marklist_reconstruct(list,&recon);

    for(j=0; j<image.h ;j++)
      for(i=0; i<image.w; i++)
	if(gpix(recon,i,j))
	  ppix(image,i,j,0);
    marktype_free(&recon);
  }

  return list;
}



marklistptr 
imagefn_to_list(marktype *image, char *fn, int *w, int *h)
{
  marklistptr list=NULL,list2=NULL;
  marktype copy;

  if(marktype_readnamed(fn, image)!=0)
    return NULL;

  *w=image->w;
  *h=image->h;

  if(image->resolution==0){
    image->resolution=globals.g_default_resolution;
  }

  copy=marktype_copy(*image);

  
  /* rotate the image, keeping original size */
  rotate_image(image, globals.g_rotate_degs_before_processing, 1);


  list=extract_marks_from_zones(*image);
  /*
  list=extract_all_marks(list,
			 image,
			 globals.g_extraction_nested,
			 globals.g_extraction_connectivity);
			 */

  list=list_bound_size(list,globals.g_bound_w,globals.g_bound_h);

  switch (globals.g_reading_order){
  case EXTRACTION_ORDER:
    break;
  case EXTRACTION_PROFILE:
    list2=sortmarks_profile(list,image->w, image->h,0,image->resolution);
    marklist_free(&list);
    list=list2;
    break;
  case EXTRACTION_RANDOM:
    list2=sortmarks_random(list,image->w, image->h,0,image->resolution);
    marklist_free(&list);
    list=list2;
    break;
  case EXTRACTION_ZHANG:
    list2=sortmarks_zhang(list,image->w, image->h,0,image->resolution);
    marklist_free(&list);
    list=list2;
    break;
  case EXTRACTION_HOWARD:{
    marklistptr pruned=NULL;

    if(globals.g_prune_under_area){
      pruned=marklist_prune_under_area(&list,globals.g_prune_under_area,image->resolution);
      /*fprintf(stderr,"pruned: %d\n",marklist_length(pruned));*/
    }

    list2=sortmarks_howard(list,image->w, image->h,0,image->resolution);
    marklist_free(&list);
    list=list2;

    if(pruned)
      pruned->data.start_of_line=START_PRUNED;
    marklist_append(&list,pruned);
    marklist_free(&pruned);
  }
    break;
  case EXTRACTION_DOCSTRUM:{
    marklistptr pruned=NULL;

    if(globals.g_prune_under_area||globals.g_prune_as_pixels){
      pruned=marklist_prune_under_area(&list,globals.g_prune_under_area,image->resolution);
      marklist_prune_over_area(&list,globals.g_prune_over_area,image->resolution);
      /*fprintf(stderr,"pruned: %d\n",marklist_length(pruned));*/
    }
    list2=docstrum_order(list);
    marklist_free(&list);
    list=list2;

    if(globals.g_prune_as_pixels){
      int i,j;
      marktype recon;
      marklist_reconstruct(list,&recon);
      
      for(j=0; j<copy.h ;j++)
	for(i=0; i<copy.w; i++)
	  if(gpix(recon,i,j)!=gpix(copy,i,j))
	    ppix((*image),i,j,gpix(copy,i,j));
      marktype_free(&recon);
    } else {
      if(pruned)
	pruned->data.start_of_line=START_PRUNED;
      marklist_append(&list,pruned);
      marklist_free(&pruned);      
    }
    marktype_free(&copy);
    

    /**/
  }
    break;
  default:
    {
      fprintf(stderr,"Unknown reading order mode.\n");
      exit(1);
    }
  }

  /*
  {
    marktype im;
    marklist_reconstruct_show_lines (list, &im);
    marktype_writenamed("a.pbm",im);
    fprintf(stderr,"wow\n");
    marktype_free(&im);
  }
  */

  return list;
}




void 
list_to_imagefn(marklistptr list, marktype recon, char *fn, int w, int h)
{
  marktype image;
  int j,i;

  marklist_reconstruct_using_wh(list, &image,w,h);
  for(j=0;j<recon.h;j++)
    for(i=0;i<recon.w;i++)
      if(gpix(recon,i,j)){
	ppix(image,i,j,1);
      }

  marktype_writenamed(fn, image);
  fprintf(stderr,"main_tic: wrote '%s'\n",fn);
  marktype_free(&image);

}




void
dump_log()
{



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
	    "  -R m\tsets the reading order (Docstrum*)\n"
	    "  -l f\tlog to file <f>\n"
	    "  -t t\tset matching threshold to <t>\n"
	    "  -Nn \t(N)ested extraction or (n)on-(n)ested extraction\n"
	    "  -48 \t4 or 8 connectivity\n"
	    "  -P  \tuse PPM to encode indices\n"
	    "  -u  \tturn off probabilities update after clairvoyant encoding\n"
	    "  -D d\tset default resolution to d DPI\n"
	    "  -C f\tdump codebook to <f> in .pbm format\n"
	    "  -M n\tsets the maximum number of classes to <n>\n"
	    "  -E n\tsets the maximum examples in each class to <n>\n"
	    "  -1 n\tSets the normal component of the clairvoyant context\n"
	    "  -2 n\tSets the clair component of the clairvoyant context\n"
	    "  -z  \tdumps the zones\n"
	    "  -f n\tfile layout filename <n>\n"
	    "  -U:6\tUpdate params: cinit,inc,norm gammainit,inc,norm\n"
	    "  -C n\tCodebook filename <n>\n"
	    "  -I  \tdump component indices\n"
	    "  -O  \trotate offsets\n"
	    "  -P  \tprime against average component before compressing it\n"
	    "  -G  \talign using centres (instead of centroids)\n"
	    "  -m n\tMatch mode B_AVG (default), F_AVG, B_SINGLE, F_SINGLE, HYBRID\n"
	    "  -Z  \tPruning values determined by machine learning\n"
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
	    "\nAuthor information:\n"
	    "\tStuart Inglis, http://www.cs.waikato.ac.nz/~singlis\n"
	    "\n"
	    "\tThesis results (Table 9.1):\n"
	    "\t    tic98 -c -i A006.pbm -o A006.cmp -Z -m B_SINGLE\n"
	    "\n"
	    "version:\n"
	    "\t1 July 1999 (version 1.01)\n"
	    );
    exit(0);
}



int 
main(int argc, char *args[])
{
  int ch;
  extern char *optarg;
  int compress=1;
  int num_in=0;
  int num_out=0;
  char **infilename=NULL;
  char **outfilename=NULL;

  globals_init();
  TimerInit();


  while((ch = getopt(argc, args, "cD:di:o:U:uPnN48l:R:w:h:s:r:p:I:SOf:zC:t:m:FE:M:b1:2:AGZ")) != -1)
    switch(ch)
      {
      case 'Z':
	globals.g_prune_as_pixels=1;
	globals.g_prune_under_area=0.5;
	globals.g_prune_over_area=5;
	break;
      case 'G':
	globals.g_align_using_centres=1;
	break;
      case 'm':
	if(strcasecmp(optarg,"B_AVG")==0) globals.g_match_mode=MATCH_B_AVG;
	else if(strcasecmp(optarg,"F_AVG")==0) globals.g_match_mode=MATCH_F_AVG;
	else if(strcasecmp(optarg,"B_SINGLE")==0) globals.g_match_mode=MATCH_B_SINGLE;
	else if(strcasecmp(optarg,"F_SINGLE")==0) globals.g_match_mode=MATCH_F_SINGLE;
	else if(strcasecmp(optarg,"HYBRID")==0) globals.g_match_mode=MATCH_HYBRID;
	else {
	  fprintf(stderr,"error: unknown matching mode option. Use B_AVG,F_AVG,B_SINGLE,F_SINGLE or HYBRID.\n");
	  exit(1);
	}
	break;
      case '1':
	globals.g_num1=atoi(optarg);
	break;
      case '2':
	globals.g_num2=atoi(optarg);
	break;
      case 'b':
	globals.g_use_missing_bit=1;
	break;
      case 'M':
	globals.g_max_classes=atoi(optarg);
	break;
      case 'E':
	globals.g_max_examples=atoi(optarg);
	break;
      case 'z':
	globals.g_dump_zones=1;
	break;
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
	  fprintf(stderr,"error: unknown Reading order mode. Use one of:\n");
	  fprintf(stderr,"\tHoward, Zhang, Extraction, Profile, Random or Docstrum\n");
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
	break;
      case 'D':
	globals.g_default_resolution=atoi(optarg);
	break;
      case 'u':
	globals.g_update_probs_after_encoding=0;
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
      case 'C':
	globals.g_codebook_filename=strdup(optarg);
	break;
      case 't':
	globals.g_match_threshold=atoi(optarg);
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

  if((infilename==NULL)||(outfilename==NULL))
    usage();

  if((compress==1) && (num_out!=1))
    usage();

  if((compress==0) && (num_in!=1) && (num_out!=1))
    usage();

  /*
   * Arithmetic coders usually read/write to stdin/stdout, but here
   * we'll reassign them to files with "b" mode for compatibility 
   * on non-unix machines 
   */

  if(compress){
    marklistptr list=NULL;
    tic98type t;
    int i;

    TimerStart(1);

    freopen(outfilename[0],"wb",stdout);

    tic98_init(&t);
    tic98_start_encoding();
    
    for(i=0;i<num_in;i++){
      int w,h,xx,yy,ok=0;
      marktype image;
      list=imagefn_to_list(&image,infilename[i], &w, &h);
      
      tic98_num_encode(&t,NEW_PAGE);

      for(xx=0;xx<image.w;xx++)
	for(yy=0;yy<image.h;yy++)
	  if(gpix(image,xx,yy))
	    ok++;

      /*fprintf(stderr,"%d\n",ok);*/

      tic98_compress_marks(&t,list, w,h);

      /*fprintf(stderr,"bl_compress\n");*/
      if(ok)ok=1;
      tic98_num_encode(&t,ok);
      if(ok){
	/*marktype_writenamed("bl.pbm",image);*/
#ifndef Q_CODER
	bl_compress(image,default_template);
#endif
      }

      marktype_free(&image);


    /* usually here... tic98_status(&t,globals.g_logfile);*/
      if(i<(num_in-1)){
	tic98_status(&t,globals.g_logfile);
      }

      marklist_stats(list,globals.g_logfile);
      marklist_free(&list);
    }

    tic98_num_encode(&t,END_OF_DOCUMENT);
    tic98_finish_encoding();

    tic98_status(&t,globals.g_logfile);

    tic98_free(&t);

    TimerStop(1);
    TimerPrint(1,globals.g_logfile);

    fclose(stdout);

    marklist_free(&list);
  } else { 
    marklistptr list=NULL;
    tic98type t;
    char *newoutname=NULL;
    int page=0;
    
    newoutname=(char*)malloc(sizeof(char)*(strlen(outfilename[0])+10));
    assert(newoutname);

    freopen(infilename[0],"rb",stdin);

    tic98_init(&t);
    tic98_start_decoding();

    while(tic98_num_decode(&t)!=END_OF_DOCUMENT){
      int w,h,ok;
      marktype recon;

      page++;
      list=tic98_decompress_marks(&t,&w,&h);
      ok=tic98_num_decode(&t);
      if(ok==1){
	recon.w=w;
	recon.h=h;
#ifndef Q_CODER
	bl_decompress(&recon);
#endif
      }

      sprintf(newoutname,"%s.%03d",outfilename[0],page);
      list_to_imagefn(list,recon,newoutname,w,h);

      marktype_free(&recon);

      marklist_free(&list);
    }

    free(newoutname);

    tic98_finish_decoding();
    tic98_free(&t);

    fclose(stdin);

  }

  free(infilename);
  free(outfilename);

  TimerFree();


  exit(0);
}
