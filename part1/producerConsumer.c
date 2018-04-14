/* CSci5103 Fall 2016
* Assignment# 4
* name: Tanmay Nalin Mehta
* student id: 5274439
* x500 id: mehta206
*/

#include<pthread.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define MAX_ITEMS 1000

#define MAX 5 // size of buffer

char buffer[MAX][40];
int start,end;

void get_time(char *op){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	int temp = tv.tv_usec; //time in microsecs
	sprintf(op, "%d", temp); //convert to string. better than itoa
}


pthread_mutex_t  lock;
pthread_cond_t  condP, condC;

void * producer (void *ptr)
{

	char *colour = (char *) ptr;
	char fileName[25];
	memset(fileName,0,sizeof(fileName));
	strcpy( fileName, "Producer_" );
	strcat(fileName,colour);
	strcat(fileName,".txt");
	FILE *fp;
	fp = fopen( fileName , "a" );

	int i;
	for ( i = 0; i< MAX_ITEMS; i++) {

		pthread_mutex_lock ( &lock);  /* Enter critical section  */
	/* While instead of if so that when 2 producers are signaled, both dont enter critical region */
		while (  (start+1)%MAX == end ){
	/* without the while surrounding the wait stmt, deadlock occurs */
			while (  pthread_cond_wait( &condP, &lock) != 0 ) ; 
		}

		char time[30];
		get_time(time);
		strcpy(buffer[start], colour);
		strcat(buffer[start], time);
		fprintf(fp,"%s\n", buffer[start] );

		start = (start + 1)%MAX;

		pthread_mutex_unlock ( &lock);
		pthread_cond_signal( &condC ); /* signal consumer */
	}

}

void * consumer (void *ptr)
{
	int i,j;
	FILE *fp;
	fp = fopen( "Consumer.txt" , "a" );

	for(j=0;j<3*MAX_ITEMS;j++) {
		pthread_mutex_lock ( &lock);    /* critical section start */
		
	/* While instead of if so that when 2 producers are signaled, both dont enter critical region */	
		while ( start == end ){
	/* without the while surrounding the wait stmt, deadlock occurs */
			while (  pthread_cond_wait( &condC, &lock) != 0 ) ; 
		}

		char data[40];
		strcpy(data, buffer[end]);
		end = (end+1)%MAX;
		fprintf(fp,"%s\n",data);

		pthread_mutex_unlock ( &lock);   /* critical seciton end */
		pthread_cond_signal( &condP );  /* signal producer */
	}
}


main( int argc, char* argv[] )
{
	pthread_t prod, cons, prod1, prod2;

	pthread_mutex_init( &lock, NULL);
	pthread_cond_init( &condP, NULL);
	pthread_cond_init( &condC, NULL);

	char *colour;
	int n;
	/*  Create producer threads */
	colour = "RED";
	if ( n = pthread_create(&prod, NULL, producer, colour)) {
		fprintf(stderr,"error in pthread_create- %s\n",strerror(n));
		exit(1);
	}
	colour = "BLACK";
	if ( n = pthread_create(&prod1, NULL, producer, colour)) {
		fprintf(stderr,"error in pthread_create- %s\n",strerror(n));
		exit(1);
	}

	colour = "WHITE";
	if ( n = pthread_create(&prod2, NULL, producer, colour)) {
		fprintf(stderr,"error in pthread_create- %s\n",strerror(n));
		exit(1);
	}

	/*  Create consumer thread */
	if (  n = pthread_create(&cons, NULL, consumer, NULL) ) {
		fprintf(stderr,"error in pthread_create - %s\n",strerror(n));
		exit(1);
	}

	/* Wait for the consumer thread*/
	if ( n = pthread_join(cons, NULL) ) {
		fprintf(stderr,"error in pthread_join - %s\n",strerror(n));
		exit(1);
	}
	return 0;

}


