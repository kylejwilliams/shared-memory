#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int process_num = 0;	// process number of slave
int max_writes = 0;		// number of times to write to file
char buffer[26];		// holds the current time when writing to file
int i = 0;				// iter
int sleep_time = 0;		// amount of time to sleep before leaving critical 
						// section

// used to make sure a valid integer was passed as an arg
int LegalInt(char *str)
{
	while (*str)
		if (!isdigit(*str++)) return 0;
	return 1;
}

// prints the current time in an hh:mm:ss format
void FormatTimeHMS(char buffer[], int buf_size)
{
	time_t timer;
	struct tm* tm_info;

	time(&timer);
	tm_info = localtime(&timer);

	strftime(buffer, buf_size, "%H:%M:%S", tm_info);
}

void PerformCriticalSection()
{
	
}
// waits for it's turn to enter the critical section, writes to a shared 
// memory variable, and kills itself
int main(int argc, char *argv[])
{
	if (LegalInt(argv[1]) == 0 || LegalInt(argv[2]) == 0)
	{
		printf("%s\n", argv[1]);
	    fprintf(stderr, "ERROR: invalid args in slave\n");
		return -1;
	}
	
	srand(getpid()); // so that each process doesn't use the same seed and 
					// generate the same sequence of random numbers
					// 

	process_num = atoi(argv[1]);
	max_writes = atoi(argv[2]);
	
	int shmid;
	key_t key;
	int *shm;

	// get the segment we named previously
	//if ((key = ftok("/tmp", 199)) == -1)
	//{
	//	perror("ftok");
	//	exit(1);
	//}
	key = 5678;

	// locate the segment
	if ((shmid = shmget(key, 27, 0666)) < 0)
	{
		perror("shmget");
		exit(1);
	}

	// attach the segment to our data space
	if ((shm = shmat(shmid, NULL, 0)) == (int *) -1)
	{
		perror("shmat");
		exit(1);
	}

	for (i = 0; i < max_writes; i++)
	{
		fprintf(stderr, "Child - %d: I am entering the critical section\n", process_num);
		
		sleep_time = rand() % 3;
		fprintf(stderr, "Child - %d: Going to sleep for %d seconds before incrementing...\n", process_num, sleep_time);
		sleep(sleep_time);
		
		// <<<<< entering critical section >>>>>
		(*shm)++; // increment the value in memory		
		FormatTimeHMS(buffer, 26);
		printf("File modified by process number %d at time %s with shared num = %d\n", process_num, buffer, *shm);
		// <<<<< leaving critical section >>>>>

		sleep_time = rand() % 3; // sleep for 0-2 seconds
		fprintf(stderr, "Child - %d: Going to sleep for %d seconds after incrementing...\n", process_num, sleep_time);
		sleep(sleep_time);

		fprintf(stderr, "Child - %d: I am leaving the critical section\n", process_num);
	}
	
	shmdt(shm);
	exit(0);
}
