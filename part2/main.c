/* CSci5103 Fall 2016
* Assignment# 4
* name: Tanmay Nalin Mehta
* student id: 5274439
* x500 id: mehta206
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
//#define _POSIX_C_SOURCE 200112L
//#define _XOPEN_SOURCE 700
#define num_of_processes 4
#define MAX 2
/*

for diff between shmget and shm_open see this discussion : http://comp.os.linux.development.apps.narkive.com/sPuJ4fI1/shmget-vs-shm-open

 */

struct sharedData
{
	int d;
	char buffer[MAX][40];
	int start,end;
	pthread_mutex_t  lock;
	pthread_cond_t  condP, condC;
};
char colours[][10] = { "RED", "WHITE", "BLACK" };

int main()
{
	int shmem_id;       /* shared memory id */
	struct sharedData *shmem_ptr;     /* pointer to shared segment */
	key_t key;          /* A key to access shared memory segments */
	int size;           /* Memory size */
	int flag;

	key = 12345;        /* shmget doesnt have well know name, unlike shm_open. So pass this key to the process needing it */
	size = 2048;        /* sharing 2k memory */
	flag = 1023;        /* 1023 = 111111111 in binary, i.e. all permissions */

	/* creating a shared memory segment */
	shmem_id = shmget (key, size, flag);
	if (shmem_id == -1)
	{
		perror ("shmget error");
		exit (1);
	}
	//  attaching the new segment into this address space.

	// shmat( id of the memory, locataion where you want the address space to get attached, flag )
	shmem_ptr = shmat (shmem_id, (void *) NULL, 1023);
	if (shmem_ptr == (void *) -1)
	{
		perror ("shmat error");
		exit (2);
	}

	pthread_mutexattr_t attrmutex;
	pthread_mutexattr_init(&attrmutex);
	pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);
	//pthread_mutexattr_settype(&attrmutex, PTHREAD_PROCESS_SHARED);
	/* Allocate memory to pmutex here. */
	/* Initialise mutex. */
	pthread_mutex_init( &shmem_ptr->lock, &attrmutex);

	pthread_condattr_t attrcond, attrcond1;
	/* Initialise attribute to condition. */
	pthread_condattr_init(&attrcond);
	pthread_condattr_setpshared(&attrcond, PTHREAD_PROCESS_SHARED);
	pthread_condattr_init(&attrcond1);
	pthread_condattr_setpshared(&attrcond1, PTHREAD_PROCESS_SHARED);

	pthread_cond_init( &shmem_ptr->condP, &attrcond);
	pthread_cond_init( &shmem_ptr->condC, &attrcond1);

	shmem_ptr->d = 12;
	/* Start children. */
	pid_t pids[num_of_processes];
	int i;
	for (i = 0; i < num_of_processes; ++i) {
		if ((pids[i] = fork()) < 0) {
			perror("fork");
			abort();
		} else if (pids[i] == 0) {
			// child process
			// passthe key to shared memory segment as a command-line parameter to child
			char keystr[10];
			sprintf (keystr, "%d", key);

			// last process created is the consumer process.
			if(i == num_of_processes-1){
				if ( execl ("./consumer", "consumer", keystr, NULL) == -1 ){ printf("error execl\n"); }
			}
			else{
				if ( execl ("./producer", "producer", keystr, colours[i], NULL) == -1 ){ printf("error execl\n"); }
			}

			exit(0); // exteremely imp. otherwise child processes will also execute fork.
		}
	}

	shmdt ( (void *)  shmem_ptr);
	/* Wait for children to exit. */
	int status;
	pid_t pid;
	int n;
	for(n=0;n<num_of_processes;n++){
		pid = wait(&status);
	}

	shmctl (shmem_id , IPC_RMID, NULL);

	return 0;
}

