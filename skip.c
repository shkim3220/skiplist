#include "hash.h"

int cnt;
FILE *fp;
Skiplist_t skiplist;

pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

void *mythread(void *arg)
{
	int i;
	char *s;
	char str[255];
#ifdef WLOCK

	while(fgets(str,255,fp)!=NULL)
	{		
		Pthread_mutex_lock(&lock2);
		strtok_r(str,"\t",&s);
		_insert(&skiplist,atoi(str),s);
		Pthread_mutex_unlock(&lock2);
	}

#else
	while(fgets(str,255,fp)!=NULL)
	{
		strtok_r(str,"\t",&s);
		_insert(&skiplist,atoi(str),s);
	}

#endif
}
 
int main(int argc, char *argv[]) {

	int i,r,num_of_thread;
	double gap;
	pthread_t *parr;
	Node_t *v;
//	Hashtable_t hash;

	time_t start=0,end=0;

	if(argc != 2)
	{
		fprintf(stderr,"usage : ./filename number_of_thread\n");
		exit(1);
	}

    init_skiplist(&skiplist);
	cnt = 1;
	num_of_thread = atoi(argv[1]);
	parr = (pthread_t *)malloc(sizeof(pthread_t)*num_of_thread);

	if(!(fp = fopen("xac","r"))){
		printf("Error!\n");
		exit(1);
	}
	
	srand(time(NULL));

	start = clock();

	for(i=0;i<num_of_thread;i++)
	{
		Pthread_create(&parr[i],NULL,mythread,NULL);
	}

	end = clock();

	for(i=0;i<num_of_thread;i++)
	{
		Pthread_join(parr[i],NULL);
	}

	gap = (double)(end-start)/(CLOCKS_PER_SEC);

	printf("Finish\n");
	_dump(&skiplist);

	printf("\n searching \n");
	for(i=100;i<1500;i++)
	{
		v = _search(&skiplist,i);
		if(v != NULL)
			printf("Final : key :%d, value : %s\n",v->key,v->data->value);
	}

	printf("%5fs\n",gap);
//	_traverse();

    return 0;
} 
