/* Hash module definitions used by both the encoder and the decoder. */

#define HASH_SIZE  1024       /* Size of hash table; */

typedef struct 
{ /* hash list record */
    unsigned int Hnumber; /* The hashed number */
    unsigned int Hnext;   /* The next number in this list */
} hashList;

typedef struct 
{ /* the hash table */
    unsigned int hashFree; /* Next unused hashList record */
    unsigned int hashSize; /* Current size of hashLists array */
    unsigned int hashVersion; /* Version number for this table */
    unsigned int hashIndex [HASH_SIZE]; /* The hash entries. */
    unsigned int hashVerno [HASH_SIZE]; /* The hash version numbers. */
    hashList *hashLists; /* For storing lists of hash records */
} hashTable;

hashTable *
createHash(hashTable *H);
void
addHash (hashTable *H, unsigned int n);
int
foundHash (hashTable *H, unsigned int n);
void
dumpHash (FILE *fp, hashTable *H);
