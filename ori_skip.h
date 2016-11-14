#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
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
#define THREAD_MAX 65

typedef struct Node_s {
    int key;                /* key used for searching */
    char data[255];     /* user data */
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

static void _dump(Skiplist_t *sl) {
    Node_t *x = sl->header;

	int i;

	for (i = sl->listlevel; i >= 1; i--) {

    	while ((x->forward !=NULL)&&(x&&x->forward[i] != sl->header))
		{
			printf("key[%d]->", x->forward[i]->key);
			x = x->forward[i];
		}
	    printf("NIL\n");
		x= sl->header;
    }

}

static int rand_level() {
    int level = 1;
    while (rand() < RAND_MAX / 2 && level < MAXLEVEL)
        level++;
    return level;
}

Status _insert(Skiplist_t *sl, int k, char *d) {

    Node_t *update[MAXLEVEL + 1]={0,};
    Node_t *x = sl->header;

    int i, level;

    for (i = sl->listlevel; i >= 1; i--) {
        while (x->forward[i]->key < k)
            x = x->forward[i];
        update[i] = x;
    }
    x = x->forward[1];
 
    if (k == x->key) {
        strcpy(x->data,d);
        return 0;
    } else {
        level = rand_level();
        if (level > sl->listlevel) {
            for (i = sl->listlevel + 1; i <= level; i++) {
                update[i] = sl->header;
            }
            sl->listlevel = level;
        }
 
        x = (Node_t *) malloc(sizeof(Node_t));
        x->key = k;
        strcpy(x->data,d);
        x->forward = (Node_t **) malloc(sizeof(Node_t*) * (level + 1));
        for (i = 1; i <= level; i++) {
            x->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = x;
        }
    }
    return 0;

}
static void _free(Node_t *x)
{
	if (x)	{
        free(x->forward);
        free(x);
		x->forward = NULL;
		x = NULL;
    }
}

int _delete(Skiplist_t *sl, int key)
{

    int i;

    Node_t *update[MAXLEVEL + 1]={0,};
    Node_t *x = sl->header;

    for (i = sl->listlevel; i >= 1; i--) {
        while ((x->forward != NULL)&&(x->forward[i]->key < key))
            x = x->forward[i];
//		if( x->forward == NULL)
//			printf("x->forward is Null\n");
	//		return 1;
		update[i] = x;
		
    }
    x = x->forward[1];

    if (x->key == key) {
        for (i = 1; i <= sl->listlevel; i++) {
            if (update[i]->forward[i] != x)
				break;
            update[i]->forward[i] = x->forward[i];
        }

        _free(x); 

        while (sl->listlevel > 1 && sl->header->forward[sl->listlevel] == sl->header)
            sl->listlevel--;
	
        return 0;
    }
    return 1;

}
