/* Hash table module. */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include "ppmd_hash.h"

#define HASH_NULL 0

unsigned int
hash(unsigned int n)
/* Returns a hash number for n. */
{
    return (n % HASH_SIZE);
    /* real simple hash function; this ensures that
       the hash entries are uniformly distributed */
}

hashTable *
createHash (hashTable *H)
/* Creates and initializes the hash table. */
{
    
    unsigned int h;

    if (H == NULL)
    {
	H = (hashTable *) malloc (sizeof(hashTable));
	H->hashLists = (hashList *) malloc (HASH_SIZE *
						   sizeof(hashList));
	H->hashSize = HASH_SIZE;
	H->hashVersion = 0;
	for (h=0; h<HASH_SIZE; h++)
	{
	    H->hashIndex [h] = HASH_NULL;
	    H->hashVerno [h] = 1;
	}
    }
    H->hashFree = 0;
    H->hashVersion++;
    return (H);
}

unsigned int
freeHash (hashTable *H, unsigned int n)
/* Return a pointer to the next free hashList record. */
{
    unsigned int h;

    h = ++H->hashFree;
    if (H->hashFree >= H->hashSize)
    { /* extend array */
	H->hashSize += HASH_SIZE;
	H->hashLists = (hashList *) realloc (H->hashLists,
		       H->hashSize * sizeof(hashList));
	if (H->hashLists == NULL)
	{
	    fprintf (stderr, "Fatal error: out of hash list space\n");
	    exit (1);
	}
    }
    assert (h < H->hashSize);
    H->hashLists [h].Hnumber = n;
    H->hashLists [h].Hnext = HASH_NULL;
    return (h);
}

void
addHash (hashTable *H, unsigned int n)
/* Adds number n into the hash table. */
{
    unsigned int h;
    unsigned int head, here;

    assert (H != NULL);

    h = hash (n);

    /* find if the number already exists */
    head = H->hashIndex [h];
    /* check whether current version is in use */
    if (H->hashVerno [h] < H->hashVersion)
	head = HASH_NULL; /* reset old list to be NULL */
    H->hashVerno [h] = H->hashVersion;

    here = head;
    while ((here != HASH_NULL) && (n != H->hashLists [here].Hnumber))
	here = H->hashLists [here].Hnext;
    if (here == HASH_NULL)
    { /* not found - add at head of list */
	here = freeHash (H, n);
	H->hashLists [here].Hnext = head;
	H->hashIndex [h] = here;
    }
}

int
foundHash (hashTable *H, unsigned int n)
/* Returns non-zero if number n is found in the hash table. */
{
    unsigned int h;
    unsigned int head, here;

    assert (H != NULL);

    h = hash (n);
    /* find if the number already exists */
    head = H->hashIndex [h];
    /* check whether current version is in use */
    if (H->hashVerno [h] < H->hashVersion)
	return (0);

    here = head;
    while ((here != HASH_NULL) && (n != H->hashLists [here].Hnumber))
	here = H->hashLists [here].Hnext;
    if (here == HASH_NULL)
	return (0);
    else
	return (1);
}

void
dumpHash (FILE *fp, hashTable *H)
/* Dumps the hash table H. */
{
    unsigned int here;
    unsigned int h;

    assert (H != NULL);

    for (h=0; h<HASH_SIZE; h++)
	if ((H->hashIndex [h] != HASH_NULL) &&
	    (H->hashVerno [h] == H->hashVersion))
	{
	    fprintf( fp, "%6d", h );
	    here = H->hashIndex [h];
	    while (here != HASH_NULL) {
		fprintf( fp, " %u", H->hashLists [here].Hnumber );
		here = H->hashLists [here].Hnext;
	    }
	    fprintf( fp, "\n" );
	}
}

/* Scaffolding for hash module

int
main()
{
    unsigned int n;
    hashTable *H;
    int i;

    H = createHash(NULL);
    for (i=0; i<16; i++)
	addHash( H, i );
    dumpHash( stdout, H );
    for (i=0; i <= 20; i++)
    {
	printf ("%3d ", i);
	if (foundHash (H, i))
	    printf ("found\n");
	else
	    printf ("not found\n");
    }

    H = createHash(H);
    for (i=0; i<32; i++)
	addHash( H, i );
    dumpHash( stdout, H );
    for (;;)
    {
	printf( "n? " );
	scanf( "%u", &n );
	if (foundHash (H, n))
	    printf ("found\n");
	else
	    printf ("not found\n");
	if (n == 999)
	    break;
    }
    H = createHash(H);
    for (;;)
    {
	printf( "n? " );
	scanf( "%u", &n );
	addHash (H, n);
	if (n == 999)
	    break;
    }
    dumpHash( stdout, H );
    for (;;)
    {
	printf( "n? " );
	scanf( "%u", &n );
	if (foundHash (H, n))
	    printf ("found\n");
	else
	    printf ("not found\n");
    }
}

*/
