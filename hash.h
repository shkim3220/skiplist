#include "skip.h"

typedef struct Entry_s{
	int key;
	Skiplist_t *list;
	struct Entry_s *next;
}Entry_t;

typedef struct Hashtable_s {
	int size;
 	struct Entry_s **table;	
}Hashtable_t;

/* Create a new hashtable. */
void ht_create(Hashtable_t *ht, int size ) { 

	int i;

	if( size < 1 ) return ;

	/* Allocate the table itself. */
	if( ( ht = malloc( sizeof( Hashtable_t ) ) ) == NULL ) {
		return;
	}

	/* Allocate pointers to the head nodes. */
	if( ( ht->table = malloc( sizeof( Entry_t * ) * size ) ) == NULL ) {
		return;
	}
	for( i = 0; i < size; i++ ) {
		ht->table[i] = NULL;
	}

	ht->size = size;
}

/* Hash a string for a particular hash table. */
int ht_hash( Hashtable_t *ht, int *key ) {

	float v = *key %ht->size;

	printf("%3.f\n",v);

	return 0;
}

/* Create a key-value pair. */
Entry_t *ht_newpair( int *key) {
	
	Entry_t *newpair;
	Skiplist_t *sl;
	
	
	if( ( newpair = malloc( sizeof( Entry_t ) ) ) == NULL ) {
		return NULL;
	}

	newpair->key = *key;

	if( ( sl = (Skiplist_t *)malloc(sizeof(Skiplist_t))) == NULL ) {
		return NULL;
	}

	newpair->list = sl;
	newpair->next = NULL;

	return newpair;
}

/* Insert a key-value pair into a hash table. */
void ht_set( Hashtable_t *ht, int *key) {

	int bin = 0;
	Entry_t *newpair = NULL;
	Entry_t *next = NULL;
	Entry_t *last = NULL;

	
	printf("ht_set : %d\n", *key);

	bin = ht_hash( ht, &(*key) );

	next = ht->table[ bin ];

	while( next != NULL &&  *key!= next->key ) {
		last = next;
		next = next->next;
	}

	/* There's already a pair.  Let's replace that string. */
	if( next != NULL && *key == next->key ) {

	} else {
		newpair = ht_newpair( key );

		/* We're at the start of the linked list in this bin. */
		if( next == ht->table[ bin ] ) {
			newpair->next = next;
			ht->table[ bin ] = newpair;

		/* We're at the end of the linked list in this bin. */
		} else if ( next == NULL ) {
			last->next = newpair;
	
		/* We're in the middle of the list. */
		} else  {
			newpair->next = next;
			last->next = newpair;
		}
	}
}

/* Retrieve a key-value pair from a hash table. */
void *ht_get( Hashtable_t *ht, int *key ) {
	int bin = 0;
	Entry_t *pair;

	bin = ht_hash( ht, key );

	/* Step through the bin, looking for our value. */
	pair = ht->table[ bin ];

}
