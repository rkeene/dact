/*
 * Author:
 *   Stuart Inglis (singlis@internz.co.nz)
 *   (c) 1998
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "getopt.h"

#include "docstrum.h"
#include "marklist.h"
#include "boundary.h"


void usage(void)
{
  fprintf(stderr,
	  "usage: \n"
	  "\tmain_skew -i filename\n"
	  "\n"
	  "information:\n"
	  "\tCalculates the skew by a k-nearest neighbour Docstrum\n"
	  "\twith Bartlett smoothing.\n"
	  "\tA positive skew moves east anti-clockwise.\n"
	  "\n"
	  "\tIf the histogram peak is too small a ? is displayed.\n"
	  "\n"
	  "visualization:\n"
	  "            |\n"
	  "            |\n"
	  "            |            __--\n"
	  "            |     ___---\n"
	  "            |__---  +15 degrees\n"
	  "        ----+------------------\n"
	  "            |(centre)\n"
	  "            |\n"
	  "version:\n"
	  "\t20 Jan 1998\n");
  exit(0);
}





int main(int argc, char *args[])
{
  int ch;
  extern char *optarg;
  char *filename=NULL;
  marktype image;
  double skew;

  while((ch = getopt(argc, args, "hcdi:")) != -1)
    switch(ch)
      {
      case 'i':
	filename=optarg;
	break;
      case 'h':
      default:
	usage();
      }

  if(filename==NULL)
    usage();
    
  if(marktype_readnamed(filename, &image)!=0)
    return 1;

  if(calc_docstrum(&image,&skew)==0)
    fprintf(stdout,"%.1f\n",skew);
  else
    fprintf(stdout,"?\n");

  marktype_free(&image);

  exit(0);
}

