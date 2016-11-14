//#include "hash.h"
#include "ori_skip.h"

FILE *fp,*fp2;
Skiplist_t skiplist;

pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

int num_of_thread;
double gap[THREAD_MAX];

void *insert_thread(void *arg)
{
	char *s;
	char str[255];
	time_t start=0,end=0;
	int *in = (int*)arg;

	start = clock();

	while(fgets(str,255,fp)!=NULL)
	{
		Pthread_mutex_lock(&lock2);
		strtok_r(str,"\t",&s);
		_insert(&skiplist,atoi(str),s);
		Pthread_mutex_unlock(&lock2);
	}
	end = clock();

	gap[*in] = (double)(end-start)/(CLOCKS_PER_SEC);

}

void *delete_thread(void *arg)
{
	char *s;
	char str[255];
	time_t start=0,end=0;
	int *in = (int*)arg;

	start = clock();

	Pthread_mutex_lock(&lock2);

	while(fgets(str,255,fp2)!=NULL)
	{
		strtok_r(str,"\t",&s);
		_delete(&skiplist,atoi(str));
	}

	Pthread_mutex_unlock(&lock2);

	end = clock();

	gap[*in] = (double)(end-start)/(CLOCKS_PER_SEC);

}

int main(int argc, char *argv[]) {

	int i,r;
	int thread_arg[65];
	double t_time=0;
	char fpath[100],fpath2[100];
	pthread_t *parr;
	pthread_attr_t attr;
	Node_t *v;
//	Hashtable_t hash;

	time_t start=0,end=0;

	if(argc < 2)
	{
		fprintf(stderr,"usage : ./filename workload number_of_thread\n");
		exit(1);
	}

	if(pthread_attr_init(&attr) !=0)
		return 1;

	if(pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED)!=0)
		return -1;

    init_skiplist(&skiplist);
	num_of_thread = atoi(argv[2]);
	parr = (pthread_t *)malloc(sizeof(pthread_t)*num_of_thread);

	srand(time(NULL));

	strcpy(fpath,"workload/");
	strcat(fpath,argv[1]);

	strcpy(fpath2,"workload/");
	strcat(fpath2,argv[1]);


	if(!(fp = fopen(fpath,"r"))){
		printf("Error!\n");
		exit(1);
	}

	if(!(fp2 = fopen(fpath2,"r"))){
		printf("Error!\n");
		exit(1);
	}


//	printf("Number of thread : %d\n",num_of_thread);

// For Insert only
/*
	for(i=0;i<num_of_thread;i++)
	{
		thread_arg[i] = i;
		Pthread_create(&parr[i],NULL,insert_thread,&thread_arg[i]);
	}
	
	for(i=0;i<num_of_thread;i++)
	{
		Pthread_join(parr[i],NULL);
	}
*/
//	_dump(&skiplist);

//	parr = (pthread_t *)malloc(sizeof(pthread_t)*num_of_thread);

// delete only
/*
	for(i=0;i<num_of_thread;i++)
	{
		Pthread_create(&parr[i],NULL,insert_thread,NULL);
	}
	
	for(i=0;i<num_of_thread;i++)
	{
		Pthread_join(parr[i],NULL);
	}

//	_dump(&skiplist);
//	printf("-------------------------------\n");
	fseek(fp,0,SEEK_SET);

	for(i=0;i<num_of_thread;i++)
	{
		thread_arg[i] = i;
		Pthread_create(&parr[i],NULL,delete_thread,&thread_arg[i]);
	}
	
	for(i=0;i<num_of_thread;i++)
	{
		Pthread_join(parr[i],NULL);
	}

//	fseek(fp,0,SEEK_SET);

//	search();
//	_dump(&skiplist);
*/
// Insert + delete

	for(i=0;i<num_of_thread;i++)
	{
		thread_arg[i] = i;
		if((thread_arg[i]%2) == 0)
			Pthread_create(&parr[i],NULL,insert_thread,&thread_arg[i]);
		else
			Pthread_create(&parr[i],NULL,delete_thread,&thread_arg[i]);
	}
	
	for(i=0;i<num_of_thread;i++)
	{
		Pthread_join(parr[i],NULL);
	}

	for(i=0;i<num_of_thread;i++)
	{
		t_time = t_time + gap[i];
	}

	printf("%5f\n",t_time);

	free((pthread_t*)parr);

    return 0;
} 
