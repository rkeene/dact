/* Encodes the input file of integers. */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "ppmd_model.h"
#include "arithcode.h"

#define MAX_LINE 128

extern int debugLevel; /* for debugging purposes */
extern int max_order; /* maximum order of model */
int max_symbol=1000;
int debugProgress = 0;

void
usage (void)
{
    fprintf (stderr,
	     "Usage: encode [options] <input-text\n"
	     "\n"
	     "options:\n"
	     "  -d n\tdebug level=n\n"
	     "  -o n\tmax order=n\n"
	     "  -m n\tmax symbol value=n\n"
	     "  -p n\tdebug progress=n\n"
	);
    exit (2);
}

void
init_arguments (int argc, char *argv[])
{
    extern char *optarg;
    /*extern int optind; */
    int opt;

    /* set defaults */
    debugLevel = 0;

    /* get the argument options */

    while ((opt = getopt (argc, argv, "d:o:p:m:")) != -1)
	switch (opt)
	{
	case 'd':
	    debugLevel = atoi (optarg);
	    break;
	case 'o':
	    max_order = atoi (optarg);
	    break;
	case 'm':
	    max_symbol = atoi (optarg);
	    break;
	case 'p':
	    debugProgress = atoi (optarg);
	    break;
	default:
	    usage ();
	    break;
	}
    for (; optind < argc; optind++)
	usage ();
}

int
getline (FILE *fp, char *s, int max)
/* Read line from FP into S; return its length (maximum length = MAX). */
{
    int i;
    char cc=0;

    i = 0;
    while ((--max > 0) && ((cc = getc(fp)) != EOF) && (cc != '\n'))
        s [i++] = cc;
    s [i] = '\0';
    if (cc == EOF)
	return (EOF);
    else
	return (i);
}

void
report (unsigned int model, int count)
{
    void report_model(); /* Report back on stats collected from the model */

    /*    fprintf( stderr, "PPM* : %d bytes", bytes_output );*/
    /*
    if (bytes_input != 0) {
	CR = (double) bytes_output / (double) bytes_input;
	fprintf( stderr, ", %.6f bpi, CF=%4.2f, integers %d", CR*8,
		 1/CR, count);
    }
    fprintf( stderr, "\n");
    */
    /*printFree( stderr );*/
    if (model)
        report_model();
}

void
encode_file(FILE *fp)
{
    unsigned int n, count;
    int len;
    char line [MAX_LINE];

    count = 0;

    InitArithEncoding();
    ppm_start_encoding (max_order, max_symbol);

    /* Read in the integers */
    while ((len = getline (fp, line, MAX_LINE)) != EOF)
    {
        count++;
	if ((debugProgress) && ((count % debugProgress) == 0))
	    report (0, count);

	sscanf (line, "%d", &n);
	
	if(n>max_symbol){
	  fprintf(stderr,"error: %d must be in the range [0..%d]\n",n,max_symbol);
	} else {
	  ppm_encode_symbol (n);
	}
	/*	bytes_input++;*/
    }
    ppm_encode_symbol( eof_symbol());

    CloseDownArithEncoding();
    report( 0, count );
}

int
main (int argc,char *argv [])
{
    init_arguments (argc, argv);

    encode_file( stdin );
    return 0;
}
