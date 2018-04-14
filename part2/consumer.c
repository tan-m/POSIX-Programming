#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include<pthread.h>
#include<string.h>

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

void * consumer (void *p)
{
	int i,j;
	FILE *fp;
	fp = fopen( "Consumer.txt" , "a" );

	for(j=0;j<3*MAX_ITEMS;j++) {
		pthread_mutex_lock ( &ptr->lock);    /* critical section start */

		/* While instead of if so that when 2 producers are signaled, both dont enter critical region */	
		while ( ptr->start == ptr->end ){
			/* without the while surrounding the wait stmt, deadlock occurs */
			while (  pthread_cond_wait( &ptr->condC, &ptr->lock) != 0 ) ; 
		}

		char data[40];
		strcpy(data, ptr->buffer[ptr->end]);
		ptr->end = (ptr->end+1)%MAX;
		fprintf(fp,"%s\n",data);

		pthread_mutex_unlock ( &ptr->lock);   /* critical seciton end */
		pthread_cond_signal( &ptr->condP );  /* signal producer */
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


	/*  Create consumer thread */
	pthread_t cons;
	int n;
	if (  n = pthread_create(&cons, NULL, consumer, NULL) ) {
		fprintf(stderr,"error in pthread_create - %s\n",strerror(n));
		exit(1);
	}

	/* Wait for the consumer thread */
	if ( n = pthread_join(cons, NULL) ) {
		fprintf(stderr,"error in pthread_join - %s\n",strerror(n));
		exit(1);
	}

	/* Done with the program, so detach the shared segment and terminate */

	shmdt ( (void *) ptr);

}
