/* PPM* model for arithmetic encoder. */
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include "arithcode.h"
#include "ppmd_hash.h"

#define INPUT_SIZE 100000	/* Size of input file buffer. */
#define TRIE_SIZE 30000 /* Maximum number of trie records. */
#define SYMBOLS_SIZE 100000 /* Maximum number of symbols records. */
#define CLIST_SIZE 100 /* Maximum size of the context list */
#define SYMBOL_SIZE 257 /* Number of symbols + 1 for eof */
#define ESCAPE (SYMBOL_SIZE) /* for encoding escape symbols */
#define CONTEXT_SIZE 8 /* Number of context lengths to store for statistics */
#define MAX_TOTAL_COUNT (1<<30) /* Max. total count used for scaling the counts */
#define MIN_ORDER -1 /* Minimum fixed order */
#define MAX_ORDER 5 /* Default maximum fixed order */
#define TCOUNT_INIT 1 /* Initial count for new node */
#define TESCAPE_INIT 1 /* Initial escape count for new node */
#define TCOUNT_INCR 2 /* Increment count for node */
#define TESCAPE_INCR 1 /* Increment escape count for node */
#define NIL 0 /* Indicates ptr is nil */

#define ROOT 1 /* root of Trie */

/* the following global variables are set from the command line arguments */
/* debugging output is redirected to stderr */
/* Levels of debugging output :
 * level 1 : trace of arithmetic coding values
 * level 2 : level 1 plus prediction output
 * level 3 : level 2 plus dump of tries and context list
 * level 4 : comprehensive trace information
 */
int debugLevel = 0; /* for various levels of debugging output */
int debugContextlengths = 0; /* for dumping out the context lengths */

unsigned int max_total_count = MAX_TOTAL_COUNT;
unsigned int max_order = MAX_ORDER;
unsigned int escape_value = ESCAPE;
unsigned int symbol_size = SYMBOL_SIZE;
unsigned int tcount_init = TCOUNT_INIT;
unsigned int tescape_init = TESCAPE_INIT;
unsigned int tcount_incr = TCOUNT_INCR;
unsigned int tescape_incr = TESCAPE_INCR;
float determ_factor = 3.0;

unsigned int *Input;			/* the input file */
unsigned int inputSize = 0;		/* Current size of the input array */
unsigned int inputPos = 0;		/* Current position in the input */
unsigned int inputLen = 0;		/* length of input */



hashTable *Exclusions;   /* For computing exclusions for each char */

typedef struct { /* context list record */
    unsigned int Cptr; /* pointer to node in the trie */
    unsigned int Cnext; /* next in the context list */
} contextType;

contextType Context [CLIST_SIZE];
/* linked list of the current active contexts, longest first */
unsigned int contextLength; /* length of the context list */
unsigned int contextUsed; /* list of deleted context list records */
unsigned int contextFree; /* next unused context list record */
unsigned int contextHead; /* Head of the context list = longest context */
unsigned int contextTail; /* Tail of the context list = shortest context */

typedef struct { /* symbol list record */
    int Sptr; /* pointer to node in the trie (positive) or input (negative) */
    unsigned int Snext; /* next symbol in the list */
    unsigned int Symbol; /* the symbol */
} symbolsType;

symbolsType *Symbols;
/* for storing lists of symbols associated with each node in the Trie */
unsigned int symbolsFree = 0; /* Next unused symbol record */
unsigned int symbolsSize = 0; /* Current size of the symbols array */

typedef struct { /* trie record */
    unsigned int Tsymbols; /* linked list of symbols & ptrs to lower levels in the trie */
    unsigned int Tcount; /* count associated with the node; sum of counts of all lower levels */
    unsigned int Tescape; /* escape count associated with the node */
} trieType;

trieType *Trie;		/* the trie */
unsigned int trieSize = 0;	/* Current size of the trie array */
unsigned int trieFree;		/* next unused trie record */

void
init_ppmd_globals()
{
  max_total_count = MAX_TOTAL_COUNT;
  max_order = MAX_ORDER;
  symbol_size = SYMBOL_SIZE;
  escape_value = ESCAPE;
  tcount_init = TCOUNT_INIT;
  tescape_init = TESCAPE_INIT;
  tcount_incr = TCOUNT_INCR;
  tescape_incr = TESCAPE_INCR;
  determ_factor = 3.0;

  /* the input file */
  inputSize = 0;		/* Current size of the input array */
  inputPos = 0;		/* Current position in the input */
  inputLen = 0;		/* length of input */

  symbolsFree = 0; /* Next unused symbol record */
  symbolsSize = 0; /* Current size of the symbols array */
  trieSize = 0;	/* Current size of the trie array */

}


void
printFree (FILE *fp)
{
   fprintf( fp, "Tries used: %d Tries unused %d, Symbols used: %d Symbols unused: %d Lists used: %d Lists unused: %d\n",
	    trieFree-1, (TRIE_SIZE-trieFree), symbolsFree-1, (SYMBOLS_SIZE-symbolsFree),
	    contextFree-1, (CLIST_SIZE-contextFree) );
} /* printFree */

void
initInput (void)
/* Initialize the input array */
{
    Input = NULL;
    Input = (unsigned int *) malloc (INPUT_SIZE * sizeof(unsigned int));
    inputSize = INPUT_SIZE;
}

unsigned int
freeInput (unsigned int pos)
/* Free space up in Input array. */
{
    if (pos >= inputSize)
    {				/* extend array */
	inputSize += INPUT_SIZE;
	Input = (unsigned int *) realloc (Input, inputSize *
					  sizeof(unsigned int));
	if (Input == NULL)
	{
	    fprintf (stderr, "Fatal error: out of input space at position %d\n",
		     pos);
	    exit (1);
	}
    }
    pos++;
    return (pos);
}

void
startInput (void)
{
    inputPos = 0;
}

void
putInput (int symbol)
/* Insert new symbol into Input. */
{
    freeInput (inputPos);
    Input [inputPos++] = symbol;
}

void
initContext()
/* Initialize the context list for the linked list of context pointers */
{
    contextUsed = 0;
    contextFree = 0;
    contextHead = 0;
    contextTail = 0;
}

unsigned int
freeContext()
/* Return a new pointer to a list record */
{
    unsigned int c;

    if (contextUsed == NIL) {
      if (contextFree < CLIST_SIZE) {
          contextFree += 1;
          c = contextFree;
      }
      else {
          fprintf( stderr, "Fatal error: out of context list space at position %d\n",
		   inputPos );
	  exit(1);
      }
    }
    else { /* use the first record on the free list */
      c = contextUsed;
      contextUsed = Context [c].Cnext;
    }

    if (c != NIL) {
	Context [c].Cptr = NIL;
	Context [c].Cnext = NIL;
    }

    return( c );
}

void
releaseList (unsigned int c)
/* Release the context list C to the free list */
{
   Context [c].Cnext = contextUsed;
   contextUsed = c;
   contextLength--;
}

void
dumpList (FILE *fp, unsigned int c)
/* Dump the context list C */
{
    fprintf( fp, "Dump of context list : " );
    while (c) {
	fprintf( fp, "%d ", Context [c].Cptr );
	c = Context [c].Cnext;
    }
    fprintf( fp, "\n" );
}

void
dumpSymbol (FILE *fp, unsigned int symbol)
/* Dump the symbol */
{
    if ((symbol <= 32) || (symbol >= 127))
        fprintf( fp, "<%d>", symbol );
    else
        fprintf( fp, "%c", symbol );
}

void
dumpString (FILE *fp, char *str, unsigned int pos, unsigned int len)
/* Dump the string STR starting at position POS. */
{
    char cc;
    unsigned int p;

    for (p = pos; p<pos+len; p++) {
	cc = str [p];
	if ((cc <= 31) || (cc >= 127))
	    fprintf( fp, "<%d>", cc );
	else
            fprintf( fp, "%c", cc );
    }
}

char dumpTrieStr [CLIST_SIZE];

void
dumpTrie(FILE *fp, int t, unsigned int d)
/* Dump the trie T; d is 0 when at the top level. */
{
    unsigned int sym, s;
    int sptr;

    fprintf( fp, "%5d %5d ", d, t );
    if (t < 0) /* pointer to input */
	fprintf( fp, "               <" );
    else {
	fprintf( fp, " %5d %5d   <", Trie [t].Tescape, Trie [t].Tcount );
    }

    dumpString( fp, dumpTrieStr, 0, d );
    fprintf( fp, ">\n" );
    if (t > 0) {
      s = Trie [t].Tsymbols;
      while (s != NIL) {
	sym = Symbols [s].Symbol;
	sptr = Symbols [s].Sptr;
	dumpTrieStr [d] = sym;
	dumpTrie( fp, sptr, d+1 );
	s = Symbols [s].Snext;
      }
    }
}

void
dumpRoot (FILE *fp)
/* Dump the root trie. */
{
    fprintf( fp, "Dump of Trie : \n" );
    fprintf( fp, "---------------\n" );
    fprintf( fp, "depth node   esc count context\n" );
    dumpTrie( fp, ROOT, 0 );
    fprintf( fp, "---------------\n" );
    fprintf( fp, "\n" );
}

void
junk()
{
    fprintf( stderr, "Got here\n" );
    /*dumpRoot( stderr );*/
}

unsigned int
getCount (unsigned int s)
/* Gets the count for the symbol at node s in the symbol list. */
{
    unsigned int freq;
    int sptr;

    if (s == NIL)
	return( 0 );

    freq = 0; /* frequency count for the symbol */
    sptr = Symbols [s].Sptr;
    assert( sptr != 0 );
    if (sptr < 0) /* points to input stream */
	freq = 1;
    else if (sptr == 0)
	freq = 0;
    else
	freq = Trie [sptr].Tcount;
    if ((debugLevel > 3) && (freq > 0)) {
	fprintf( stderr, "getCount for symbol " );
	dumpSymbol( stderr, Symbols [s].Symbol );
	fprintf( stderr, " at node %d = %d\n", sptr, freq );
    }
    return( freq );
}

unsigned int
freeTrie()
/* Return a pointer to the next free trie record. */
{
    unsigned int t;

    if (trieFree >= trieSize)
    {				/* extend array */
	trieSize += TRIE_SIZE;
	Trie = ( trieType *) realloc (Trie, trieSize *
					    sizeof ( trieType));
	if (Trie == NULL)
	{
	    fprintf (stderr, "Fatal error: out of trie space at position %d\n",
		     inputPos);
	    exit (1);
	}
    }
    t = trieFree;
    assert (t < trieSize);
    Trie [t].Tsymbols = NIL;
    Trie [t].Tcount = 0;
    Trie [t].Tescape = 0;
    trieFree++;
    return (t);
}

void
initTrie()
/* Initialize the trie. */
{
    Trie = NULL;
    Trie = ( trieType *) malloc (TRIE_SIZE * sizeof (trieType));
    trieSize = TRIE_SIZE;
    trieFree = ROOT + 1;
    Trie[ROOT].Tsymbols = NIL;
    Trie[ROOT].Tcount = 0;
    Trie[ROOT].Tescape = 0;
}

void
initSymbols()
/* Initialize the symbols array. */
{
    Symbols = NULL;
    Symbols = (symbolsType *) malloc (SYMBOLS_SIZE * sizeof(symbolsType));
    symbolsSize = SYMBOLS_SIZE;
    symbolsFree = 1;
}

unsigned int
freeSymbol (unsigned int symbol)
/* Return a pointer to the next free symbols record. */
{
    unsigned int s;

    if (symbolsFree >= symbolsSize)
    { /* extend array */
	symbolsSize += SYMBOLS_SIZE;
	Symbols = (symbolsType *) realloc (Symbols, symbolsSize
					   * sizeof(symbolsType));
	if (Symbols == NULL)
	{
	    fprintf (stderr, "Fatal error: out of symbols space at position %d\n",
		     inputPos);
	    exit (1);
	}
    }
    s = symbolsFree;
    assert (s < symbolsSize);
    Symbols [s].Sptr = NIL;
    Symbols [s].Snext = NIL;
    Symbols [s].Symbol = symbol;
    symbolsFree++;
    return( s );
}

unsigned int
findSymbol (unsigned int t, unsigned int symbol)
/* Return a pointer to the trie node that contains the symbol. */
{
    unsigned int s, sym;

    s = Trie [t].Tsymbols;
    while (s != NIL) {
	sym = Symbols [s].Symbol;
	if (sym == symbol)
	    return( s );
	s = Symbols [s].Snext;
    }
    return( NIL );
}

unsigned int
addSymbol (unsigned int t, unsigned int symbol)
/* Add a new symbol if required and return a pointer to it. */
{
    unsigned int s;

    s = findSymbol( t, symbol ); /* does the symbol already exist? */
    if (!s) { /* symbol not found */
      /* create new node for symbol */
	s = freeSymbol( symbol );
	if (Trie [t].Tsymbols == 0) /* empty list */
	    Trie [t].Tsymbols = s;
	else { /* add the new symbol at the head of the list */
	    Symbols [s].Snext = Trie [t].Tsymbols;
	    Trie [t].Tsymbols = s;
	}
    }
    return( s );
}

void
startContext()
{
    unsigned int c;

    assert (contextLength <= max_order+1);

    /* create new context pointing to root of trie for the new symbol */
    c = freeContext();
    Context [c].Cptr = ROOT;
    Context [c].Cnext = NIL;
    /* now add new context at tail of the contexts list */
    if (contextTail == NIL)
        contextHead = c;
    else
        Context [contextTail].Cnext = c;
    contextTail = c;
    contextLength++;
    if (debugLevel > 2) {
        fprintf( stderr, "\n" );
        dumpList( stderr, contextHead );
    }
}

unsigned int
predict()
/* Make a prediction of the next symbol. Return a pounsigned inter to the
   predicted context node. */
{
    unsigned int context;

    context = contextHead; /* start at the longest context */
    if ((context != NIL) && (contextLength-1 > max_order))
	context = Context [context].Cnext; /* go to next context */
    return( context );
}

void
arith_encode (unsigned int lbnd, unsigned int hbnd, unsigned int totl)
/* Arithmetically encode the range. */
{
    if (totl > max_total_count ) {
        fprintf( stderr, "Fatal error - totl too big : lbnd = %d, hbnd = %d, totl = %d at position %d\n",
		 lbnd, hbnd, totl, inputPos );
	/*dumpRoot( stderr );*/
    }
    assert( totl <= max_total_count );

    if ((lbnd == 0) && (hbnd == totl)) {
	/* probility = 1 - no need to encode it */
	if (debugLevel > 0)
	    fprintf( stderr, "probability = 1\n" );
    }
    else
	arithmetic_encode( lbnd, hbnd, totl );
}

#ifdef 0
unsigned int
arith_decode_target (unsigned int totl)
/* Arithmetically decodes the target. */
{
    unsigned int target;

    target = arithmetic_decode_target( totl );
    if (debugLevel > 1)
        fprintf( stderr, "target = %d, totl = %d\n", target, totl );
    return( arithmetic_decode_target( totl ));
}

void
arith_decode(unsigned int lbnd, unsigned int hbnd, unsigned int totl)
/* Arithmetically decode the range. */
{
    if ((lbnd == 0) && (hbnd == totl)) {
        /* probility = 1 - no need to decode it */
        if (debugLevel > 0)
            fprintf( stderr, "probability = 1\n" );
    }
    else {
        if (debugLevel > 0)
            fprintf( stderr, "lbnd = %d, hbnd = %d, totl = %d\n",
		     lbnd, hbnd, totl );
        arithmetic_decode( lbnd, hbnd, totl );
    }
}
#endif

void
encode_counts (unsigned int t, unsigned int symbol,
	        hashTable *exclusions)
/* Encode the symbol using the counts at node t in the trie. Do not include any
   of the exclusions. (If t is NIL, then encode using order-1 frequencies). */
{
    unsigned int s, sym, freq;
    unsigned int lbnd, hbnd, totl, determ;

    if (debugLevel > 1) {
	if (symbol == escape_value)
	    fprintf( stderr, "Encoding escape symbol\n" );
	else {
	    fprintf( stderr, "Encoding symbol (%d) ", symbol );
	    dumpSymbol( stderr, symbol );
	    fprintf( stderr, "\n" );
	}
    }

    lbnd = 0; hbnd = 0; totl = 0;
    if (t == NIL) { /* order-1 */
	for (sym = 0; sym<symbol_size; sym++) {
	    if (!foundHash (exclusions, sym)) {
		freq = 1; /* set order-1 frequency to 1 */
		if (sym == symbol) {
		    lbnd = totl;
		    hbnd = lbnd + freq;
		}
		totl += freq;
	    }
	}
    }
    else {
	lbnd = 0;
	totl = Trie [t].Tescape; /* add in escape count */
	if (!totl) { /* escape count must be non-zero, except at pos 0 */
	    assert( inputPos == 0 );
	    return;
	}
	determ = (totl == 1);
	hbnd = totl;
	s = Trie [t].Tsymbols;
	while (s != NIL) {
	    sym = Symbols [s].Symbol;
	    if (!foundHash (exclusions, sym)) {
		freq = getCount( s );
		if ((determ) && (freq > 1))
		    freq = (int) (freq * determ_factor);
		if (sym == symbol) {
		    lbnd = totl;
		    hbnd = lbnd + freq;
		}
		totl += freq;
	    }
	    s = Symbols [s].Snext;
	}
    }

    if ((hbnd <= lbnd) || (totl < hbnd)) {
        fprintf( stderr, "Fatal error - invalid range : lbnd %d hbnd %d totl %d at position %d\n",
		 lbnd, hbnd, totl, inputPos );
	fprintf( stderr, "Dumping node %d\n", t );
	dumpTrie( stderr, t, 0 );
    }
    assert( hbnd > lbnd );
    assert( totl >= hbnd );

    arith_encode( lbnd, hbnd, totl );

    if (totl > max_total_count) {
        fprintf( stderr, "Check range : lbnd %d hbnd %d totl %d at position %d\n",
		 lbnd, hbnd, totl, inputPos );
	fprintf( stderr, "Dumping node %d\n", t );
	dumpTrie( stderr, t, 0 );
	exit(1);
    }
    if (debugLevel > 0)
	fprintf( stderr, "lbnd = %d hbnd = %d totl = %d\n", lbnd, hbnd, totl );
}

unsigned int
decode_counts (unsigned int t,  hashTable *exclusions)
/* Decode the symbol at node t in the trie. Do not include any of the exclusions. */
{
    unsigned int s, sym, freq, target, determ;
    unsigned int lbnd, hbnd, totl;

    /* Find the total count first */
    if (t == NIL) { /* order-1 */
	totl = 0;
	for (sym = 0; sym<symbol_size; sym++) {
	    if (!foundHash (exclusions, sym))
	        totl += 1; /* order-1 frequency = 1 */
	}
    }
    else {
	totl = Trie [t].Tescape;
	if (!totl) { /* escape count must be non-zero, except at pos 0 */
	    assert( inputPos == 0 );
	    return( escape_value );
	}
	determ = (totl == 1);
	s = Trie [t].Tsymbols;
	while (s != NIL) {
	    sym = Symbols [s].Symbol;
	    if (!foundHash (exclusions, sym)) {
		freq = getCount( s );
		if ((determ) && (freq > 1))
		    freq = (unsigned int) (freq * determ_factor);
		totl += freq;
	    }
	    s = Symbols [s].Snext;
	}
    }
    target = arithmetic_decode_target( totl );

    /* Find the symbol and arithmetic range which target falls into */
    if (t == NIL) { /* order-1 */
	hbnd = 0;
	for (sym = 0; (sym<symbol_size) && (hbnd <= target); sym++) {
	    if (!foundHash (exclusions, sym))
	        hbnd += 1; /* order-1 frequency = 1 */
	}
	lbnd = hbnd - 1;
	sym--;
    }
    else {
	sym = escape_value; /* first symbol is an escape */
	hbnd = Trie [t].Tescape;
	determ = (hbnd == 1);
	freq = hbnd;
	s = Trie [t].Tsymbols;
	while ((s != NIL) && (hbnd <= target)) {
	    sym = Symbols [s].Symbol;
	    if (!foundHash (exclusions, sym)) {
	        freq = getCount( s );
		if ((determ) && (freq > 1))
		    freq = (unsigned int) (freq * determ_factor);
		hbnd += freq;
	    }
	    s = Symbols [s].Snext;
	}
	lbnd = hbnd - freq;
    }

    if (hbnd <= lbnd)
        fprintf( stderr, "Fatal error - invalid range : lbnd %d hbnd %d totl %d at position %d\n",
		 lbnd, hbnd, totl, inputPos );
    assert( hbnd > lbnd );
    arithmetic_decode( lbnd, hbnd, totl );

    if (debugLevel > 0) {
	if (sym == escape_value)
	    fprintf( stderr, "Decoded escape symbol\n" );
	else {
	    fprintf( stderr, "Decoded symbol (%d) ", sym );
	    dumpSymbol( stderr, sym );
	    fprintf( stderr, "\n" );
	}
    }
    return( sym );
}

void
encode_context (unsigned int context, unsigned int symbol)
/* Encode the symbol at the context in the trie. */
{
    unsigned int s, sym, cptr, freq;

    /* set up the exclusions hash table */
    Exclusions = createHash (Exclusions);

    while (context > NIL) { /* encode escapes until we find a context that has the symbol */
	cptr = Context [context].Cptr;
	s = findSymbol( cptr, symbol ); /* get the symbol's node for the context */
	freq = getCount( s );
	if (freq != 0)
	    break;

	/* Symbol not found in current context; escape to shorter contexts */
	encode_counts( cptr, escape_value, Exclusions );
        Trie [cptr].Tescape += tescape_incr;

	/* exclude all symbols in current context from being counted in shorter contexts */
	s = Trie [cptr].Tsymbols;
	while (s != NIL) {
	    sym = Symbols [s].Symbol;
	    addHash (Exclusions, sym); /* add sym to list of exclusions */
	    s = Symbols [s].Snext;
	}
        context = Context [context].Cnext; /* Go to next longest context */
    }
    if (context == NIL)
	cptr = NIL;
    else
	cptr = Context [context].Cptr; /* get the node associated with the context */
    encode_counts (cptr, symbol, Exclusions);
}

unsigned int
decode_context (unsigned int context)
/* Decode the symbol based on the context at node t. */
{
    unsigned int s, sym, cptr, symbol;

    /* set up the exclusions hash table */
    Exclusions = createHash (Exclusions);

    for (;;) { /* keep decoding until no more escapes */
	cptr = Context [context].Cptr; /* get the node associated with the context */
	
	symbol = decode_counts (cptr, Exclusions);
	if (symbol != escape_value)
	    return( symbol );

        Trie [cptr].Tescape += tescape_incr;

	/* exclude all symbols in current context from being counted in shorter contexts */
	s = Trie [cptr].Tsymbols;
	while (s != NIL) {
	    sym = Symbols [s].Symbol;
	    addHash (Exclusions, sym); /* add sym to list of exclusions */
	    s = Symbols [s].Snext;
	}

        context = Context [context].Cnext; /* Go to next longest context */
    }
}

unsigned int
nextContext (unsigned int context)
/* Return a pointer to the next context. */
{
    return( Context [context].Cnext );
}

unsigned int
dropContext(unsigned int context)
/* Drop the context from the context list, and return a pointer to the next
   in the list. */
{
    if (context != contextHead) {
	printFree( stderr );
	fprintf( stderr, "Fatal error at position %d\n", inputPos );
    }
    assert (context == contextHead); /* must be at head of context list */

    /* remove this context from the context list */
    contextHead = Context [context].Cnext;
    if (contextHead == NIL)
        contextTail = NIL;
    releaseList( context );
    context = contextHead;

    return( context );
}

unsigned int
upd (unsigned int context, unsigned int symbol, unsigned int contextLen,
     unsigned int updatecnt)
/* Update the symbol in the context. Return the next context. */
{
    unsigned int s, cptr, snew;
    int p, tnew;

    cptr = Context [context].Cptr;
    assert( cptr > NIL );

    if (updatecnt) /* this test performs update exclusions */{
        Trie [cptr].Tcount += tcount_incr; /* Only add the count for the longest context */
    }

    if (contextLen-1 > max_order)
        return( dropContext( context ));

    s = addSymbol( cptr, symbol );
    p = Symbols [s].Sptr;
    if (p > 0) { /* in middle of trie */
	Context [context].Cptr = p; /* replace context with next level down */
	return( nextContext(context) ); /* move to next context */
    }
    else if (p == 0) { /* context has not occurred before */
	Symbols [s].Sptr = - (inputPos);
        return( dropContext(context) ); /* new context, so drop from context list */
    }
    else { /* p < 0; previous context points to input stream; add it one level down */
	tnew = freeTrie();
	Trie [tnew].Tcount = tcount_init; /* set count to 1 */
	Trie [tnew].Tescape = tescape_init; /* set escape count to 1 */
	Symbols [s].Sptr = tnew;
	if (contextLen-1 < max_order) {
	    snew = addSymbol( tnew, Input [-p] ); /* next one in input */
	    Symbols [snew].Sptr = p-1; /* point to next one back in input */
	}
	Context [context].Cptr = tnew; /* replace context with new ptr */
	return( nextContext(context) ); /* move to next context */
    }
} /* upd */

void
update (unsigned int symbol)
/* Update the Trie ure using the incoming symbol at the current input position. */
{
    unsigned int context, contextLen, updatecnt;

    /*inputPos = putInput (inputPos, symbol);*/
    /*if (inputPos >= INPUT_SIZE) {
        printFree( stderr );
	fprintf( stderr, "Fatal error: out of file space at position %d\n",
		 inputPos );
	exit(1);
    }
    Input [inputPos++] = symbol;
    */
    putInput (symbol); /* store the input symbol */


    /* Update all the current contexts */
    context = contextHead;
    contextLen = contextLength;
    updatecnt = 1;
    while (context != NIL) {
	context = upd( context, symbol, contextLen, updatecnt );
	contextLen--;
	updatecnt = 0;
    }
} /* update */

void
init_symbols()
/* Initialize the model. */
{
    initInput ();
    initContext();
    initSymbols();
    initTrie();

    Exclusions = NULL;
}

unsigned int
eof_symbol()
/* Return the end of file symbol */
{
    return( symbol_size-1 );
}

void
ppm_encode_symbol (unsigned int symbol)
/* Encode the symbol. */
{
    unsigned int pContext; /* node in trie predicted to have "best" context */

    assert (symbol <= symbol_size);

    /*if ((inputPos % 10000) == 0)
      fprintf( stderr, "pos %d context %d symbols %d trie %d\n",
	       inputPos, contextFree, symbolsFree, trieFree );*/
    if (debugLevel > 1) {
        fprintf( stderr, "Input " );
	dumpSymbol( stderr, symbol );
        fprintf( stderr, " at %d\n", inputPos );
    }

    startContext();
    pContext = predict(); /* Choose the context */

    /* Encode the symbol based on the predicted context */
    encode_context( pContext, symbol );

    update (symbol);

    if (debugLevel > 2)
        dumpRoot( stderr );
}

unsigned int
ppm_decode_symbol()
/* Return the decoded symbol. */
{
    unsigned int pContext; /* node in trie with the predict "best" context */
    unsigned int symbol;

    startContext();
    pContext = predict(); /* Choose the context */

    /* Find the symbol based on the predicted context */
    symbol = decode_context( pContext );

    assert (symbol <= symbol_size);

    update( symbol );

    if (debugLevel > 2)
        dumpRoot( stderr );

    return( symbol );
}

void
report_model()
/* Report back on statistics collected during encoding */
{
    printFree( stderr );
}

void
ppm_start_encoding (unsigned int max_order1, unsigned int symbol_size1)
{
    assert (symbol_size1 > 1);
    max_order = max_order1;
    symbol_size = symbol_size1 + 2; /* add 1 for EOF symbol+esc */
    escape_value = symbol_size1 +2;

    init_symbols();

    /*    start_encode();
    startoutputtingbits();*/
    startInput ();
}

void
ppm_finish_encoding ()
{
  /*    ppm_encode_symbol( eof_symbol());*/
    /*finish_encode();
    doneoutputtingbits();*/
}

unsigned int
ppm_start_decoding (unsigned int max_order1, unsigned int symbol_size1)
/* Returns the EOF symbol. */
{
    assert (symbol_size1 > 1);
    max_order = max_order1;
    symbol_size = symbol_size1 + 2; /* add 1 for EOF symbol */
    escape_value = symbol_size1 +2;

    init_symbols();

    /*start_decode();
    startinputtingbits();*/

    return (eof_symbol());
}

void
ppm_finish_decoding ()
{
  /*    finish_decode();
    doneinputtingbits();*/
}
