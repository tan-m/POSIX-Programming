#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include<pthread.h>
#include<string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_ITEMS 1000
#define MAX 2 // size of buffer
struct sharedData *ptr; /* pointer to shared memory */

struct sharedData
{
	int d;
	char buffer[MAX][40];
	int start,end;
	pthread_mutex_t  lock;
	pthread_cond_t condP, condC;
};

void get_time(char *op){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	int temp = tv.tv_usec; //time in microsecs
	sprintf(op, "%d", temp); //convert to string. better than itoa
}

void * producer (void *p)
{
	char *colour = (char *) p;
	char fileName[25];
	memset(fileName,0,sizeof(fileName));
	strcpy( fileName, "Producer_" );
	strcat(fileName,colour);
	strcat(fileName,".txt");
	FILE *fp;
	fp = fopen( fileName , "a" );

	int i;
	for ( i = 0; i< MAX_ITEMS; i++) {

		pthread_mutex_lock ( &ptr->lock);  /* Enter critical section  */
		/* While instead of if so that when 2 producers are signaled, both dont enter critical region */
		while (  (ptr->start+1)%MAX == ptr->end ){
			/* without the while surrounding the wait stmt, deadlock occurs */
			while (  pthread_cond_wait( &ptr->condP, &ptr->lock) != 0 ) ; 
		}

		char time[30];
		get_time(time);
		strcpy(ptr->buffer[ptr->start], colour);
		strcat(ptr->buffer[ptr->start], time);
		fprintf(fp,"%s\n", ptr->buffer[ptr->start] 	);

		ptr->start = (ptr->start + 1)%MAX;

		pthread_mutex_unlock ( &ptr->lock);
		pthread_cond_signal( &ptr->condC ); /* signal consumer */
	}

}

main(int argc, char *argv[])
{
	int id;         /* shared memory identifier */

	// first parameter is the key passed.
	//shmget( key, size here is 0, flag - segment not to becreated, it exists  )	
	//id = shmget (atoi(argv[1]), 0, IPC_ALLOC); // IPC_ALLOC doesnt work on linux machines. Not widely implemented.

	id = shmget (atoi(argv[1]), 0, 0666);
	if (id == -1)
	{
		perror ("child shmget failed");
		exit (1);
	}


	//  attaching the new segment into this address space.
	// shmat( id of the memory, locataion where you want the address space to get attached, flag ) 
	ptr = shmat (id, (void *) NULL, 1023);
	if (ptr == (void *) -1)
	{
		perror ("child shmat failed");
		exit (2);
	}


	pthread_t prod;
	int n;
	/*  Create producer thread */
	if ( n = pthread_create(&prod, NULL, producer, argv[2])) {
		fprintf(stderr,"error in pthread_create- %s\n",strerror(n));
		exit(1);
	}
	/* Wait for the producer thread*/
	if ( n = pthread_join(prod, NULL) ) {
		fprintf(stderr,"error in pthread_join - %s\n",strerror(n));
		exit(1);
	}

	/* Done with the program, so detach the shared segment and terminate */

	shmdt ( (void *) ptr);

}
