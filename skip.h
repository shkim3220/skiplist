#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <sys/syscall.h>
#include "mythreads.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
/* implementation dependent declarations */
typedef enum Status{
    STATUS_OK=0,
    STATUS_MEM_EXHAUSTED =1,
    STATUS_DUPLICATE_KEY = 2,
    STATUS_KEY_NOT_FOUND = 3,
} Status;

#define compLT(a,b) (a < b)
#define compEQ(a,b) (a == b)

/* levels range from (0 .. MAXLEVEL) */
#define MAXLEVEL 15

typedef struct Data_s {
	char value[255];
} Data_t;

typedef struct Node_s {
	unsigned int valid;
    int key;                /* key used for searching */
    struct Data_s *data;     /* user data */
    struct Node_s **forward; /* skip list forward pointer */
} Node_t;

/* implementation independent declarations */
typedef struct Skiplist_s {
    struct Node_s *header;              /* list Header */
    int listlevel;              /* current level of list */
} Skiplist_t;

int init_skiplist(Skiplist_t *sl) {

    int i;
	
    Node_t *header = (Node_t *) malloc(sizeof(Node_t));
    sl->header = header;
    header->key = INT_MAX;
    header->forward = (Node_t **) malloc(sizeof(Node_t*) * (MAXLEVEL + 1));
    for (i = 0; i <= MAXLEVEL; i++) {
        header->forward[i] = sl->header;
    }
 
    sl->listlevel = 1;

	return 1;
}

inline void MemoryBarrier() {
  __asm__ __volatile__("" : : : "memory");
}

static void _dump(Skiplist_t *sl) {
    Node_t *x = sl->header;

	int i;
/*
	for (i = sl->listlevel; i >= 1; i--) {
    	while (x&&x->forward[i] != sl->header)
		{
			printf("key[%d]->", x->forward[i]->key);
        	x = x->forward[i];
		}
		
    printf("NIL\n");
	x= sl->header;
     
    }
*/
	while (x&&x->forward[1] != sl->header)
		{
			printf("key[%d]\n", x->forward[1]->key);
        	x = x->forward[1];
		}
		
    printf("NIL\n");


}

static int rand_level() {
    int level = 1;
    while (rand() < RAND_MAX / 2 && level < MAXLEVEL)
        level++;
    return level;
}

Node_t *_search(Skiplist_t *sl, int key)
{
    Node_t *x = sl->header;
    int i;
    for (i = sl->listlevel; i >= 1; i--) {
        while (x->forward[i]->key < key)
            x = x->forward[i];
		
		if(x->forward[i]->key == key)
		{
			if(x->forward[i]->valid == 1)
				return x->forward[i];
			else
				return NULL;
		}
    }
    return NULL;
}

Node_t *update_node(int k, int level, char *d)
{
		Node_t *tmp;
		Data_t *newdata = (Data_t *)malloc(sizeof(Data_t));
		strcpy(newdata->value,d); 
        tmp = (Node_t *) malloc(sizeof(Node_t));
	    tmp->key = k;
		tmp->data = newdata;
		tmp->valid = 1;
		tmp->forward = (Node_t **)malloc(sizeof(Node_t*) * (level +1));

		return tmp;
}

Status _insert(Skiplist_t *sl, int k, char *d) {

    Node_t *update[MAXLEVEL + 1];
    Node_t *x;
	Node_t *tmp;
    int i, level;

	x = sl->header;
	
	Pthread_mutex_lock(&lock);
    for (i = sl->listlevel; i >= 1; i--) {
        while (x->forward[i]->key < k)
            x = x->forward[i];
        update[i] = x;
    }
	Pthread_mutex_unlock(&lock);
	x = x->forward[1];

	// duplicate
    if (k == x->key) {
		Data_t *newdata = (Data_t *)malloc(sizeof(Data_t));		
		strcpy(newdata->value,d);
		x->data = newdata;
        return STATUS_OK;
    } else {	
        level = rand_level();

        if (level > sl->listlevel) {
            for (i = sl->listlevel + 1; i <= level; i++) {
                update[i] = sl->header;
            }
            sl->listlevel = level;
        }	

		Pthread_mutex_lock(&lock);
		tmp = update_node(k,level,d);
		Pthread_mutex_unlock(&lock);

#ifdef WLOCK
//		Pthread_mutex_lock(&lock);
		for(i = 1; i <= level; i++) {
	    	tmp->forward[i] = update[i]->forward[i];
	        update[i]->forward[i] = tmp;
	 	}
//		Pthread_mutex_unlock(&lock);
#else
		Pthread_mutex_lock(&lock);

		for (i = 1; i <= level; i++) {
//			Pthread_mutex_lock(&lock);

			tmp->forward[i] = update[i]->forward[i];
			update[i]->forward[i] = tmp;		

//			Pthread_mutex_unlock(&lock);
//			goto retry;
	 	}

		Pthread_mutex_unlock(&lock);
#endif
    }
//	_dump(sl);
    return STATUS_OK;
}
/*
static void _free(Node_t *x)
{
    if (x) {
        free(x->forward);
        free(x);
    }
}
*/
int _delete(Skiplist_t *sl, int key)
{
    int i;
    Node_t *update;
    Node_t *x = sl->header;
    for (i = sl->listlevel; i >= 1; i--) {
        while (x->forward[i]->key < key)
            x = x->forward[i];
		if(x->forward[i]->key == key)
		{
	        update = x;
			break;
		}
    }

    x = x->forward[1];
    if (x->key == key) {
		x->valid = 0;
    }
    return 1;
}
