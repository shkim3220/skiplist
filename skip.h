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

static void _dump(Skiplist_t *sl) {
    Node_t *x = sl->header;

	int i;

	for (i = sl->listlevel; i >= 1; i--) {
    	while (x&&x->forward[i] != sl->header)
		{
			if(x->forward[i]->valid == 1)
			{
				printf("key[%d]->", x->forward[i]->key);
			}
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
			{
				return x->forward[i];
			}
			else if(x->forward[i]->valid == 0)
			{
				return NULL;
			}
		}
    }
    return NULL;
}

void make_node(int key, int level,Node_t **node, Data_t **data,  char *d)
{
		*node = (Node_t *)malloc(sizeof(Node_t));
		*data = (Data_t *)malloc(sizeof(Data_t));

		strcpy((*data)->value,d); 
	    (*node)->key = key;
		(*node)->data = *data;
		(*node)->valid = 1;
		(*node)->forward = (Node_t **)malloc(sizeof(Node_t*) * (level +1));

/*
		strcpy((*data)->value,d); 
	    node->key = key;
		node->data = *data;
		node->valid = 1;

		return node;
*/
}


int CAS(Node_t **cur_node, Node_t *old_node, Node_t **new_node)
{
	int ret,f=0;
//	printf("1(In CAS) cur : %p, old : %p, new %p\n",*cur_node,old_node,*new_node);

	__asm__ __volatile__(  
            "LOCK\n\t"  
            "CMPXCHG %3, %0\n\t"
			"jnz DONE\n\t"
            "movl %4, %1\n\t " 
			"DONE:\n\t" 
            :"=m"(*cur_node),"=g"(ret)  
            :"a" (old_node),"r" (*new_node),"r"(f),"m"(*cur_node)  
            :"memory" 
            );

//	printf("2(In CAS) cur : %p, old : %p, new %p\n",*cur_node,old_node,*new_node);

//	printf("%d\n",ret);

	return ret;
}
Status _insert(Skiplist_t *sl, int k, char *d) {

    Node_t *update[MAXLEVEL + 1] = { 0 };
	Node_t *expected[MAXLEVEL + 1] = { 0 };
    Node_t *x,*node;
	Data_t *data;
    int i, level;

	level = rand_level();

	if(level > sl->listlevel)
	{
		for(i= sl->listlevel +1; i <= level; i++)
		{
			update[i] = sl->header;
		}
		sl->listlevel = level;
	}

//	node->forward = (Node_t **)malloc(sizeof(Node_t*) * (level +1));

retry :

	make_node(k,level,&node,&data,d);

	x = sl->header;

    for (i = sl->listlevel; i >= 1; i--) {
        while (x->forward[i]->key < k)
            x = x->forward[i];
        update[i] = x;
		expected[i]= x->forward[i] ;
    }

	x = x->forward[1];

	// duplicate
    if (k == x->key) {
		if(x->valid == 0)
			x->valid = 1;
		strcpy(data->value,d);
		x->data = data;
        return STATUS_OK;
    } else {	

		for (i = 1; i <= level; i++) {

			node->forward[i] = update[i]->forward[i];

			if((update[i] == 0x0)||(CAS(&(update[i]->forward[i]),expected[i],&node))==0?0:1)
			{
//				printf("%p , Retry\n",update[i]);
				goto retry;
			}
	 	}
    }

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
	        update = x->forward[i];
		}
    }


    if (update->key == key) {
		update->valid = 0;
		return 0;
    }

	return 1;
}
