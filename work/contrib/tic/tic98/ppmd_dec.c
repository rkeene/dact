/* Decodes the encoded input file. */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "ppmd_model.h"
#include "arithcode.h"

extern int debugLevel; /* for debugging purposes */
extern unsigned int max_order; /* maximum order of model */
int debugProgress = 0;
int max_symbol = 1000;

void
usage (void)
{
    fprintf (stderr,
	     "Usage: encode [options] <input-text\n"
	     "\n"
	     "options:\n"
	     "  -d n\tdebug level=n\n"
	     "  -m n\tsymbol maximum=n\n"
	     "  -o n\tmax order=n\n"
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
	case 'm':
	    max_symbol = atoi (optarg);
	    break;
	case 'd':
	    debugLevel = atoi (optarg);
	    break;
	case 'o':
	    max_order = atoi (optarg);
	    break;
	case 'p':
	    debugProgress = atoi (optarg);
	    break;
	default:
	    usage();
	    break;
	}
    for (; optind < argc; optind++)
	usage ();
}

void
report()
{
    void report_model(); /* Report back on stats collected from the model */

    /*fprintf( stderr, "\n");
    report_model();*/
}

void
decode_file (FILE *fp)
{
    unsigned int symbol, eof_sym, count;

    count = 0;

    InitArithDecoding();
    init_ppmd_globals();
    eof_sym = ppm_start_decoding (max_order, max_symbol);

    while ((symbol = ppm_decode_symbol()) < eof_sym)
    {
	count++;
	if ((debugProgress) && ((count % debugProgress) == 0))
	    fprintf (stderr, "decoded %d bytes...\n", count);

        fprintf (fp, "%u\n", symbol);
    }
    CloseDownArithDecoding();
    report();
}

int
main(int argc,char *argv[])
{
    init_arguments (argc, argv);

    decode_file (stdout);
    return 0;
}
