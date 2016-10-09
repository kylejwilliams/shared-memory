#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int child_num = 0;		// rank of the child
int process_num = 0;	// process number of slave
int max_writes = 0;		// number of times to write to file
int num_slaves = 0;		// number of sister process, including the current one
char buffer[26];		// holds the current time when writing to file
int i = 0;				// iter
int j = 0;				// iter
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

// perform the critical section for the num_writeth time and return the remaining number of times to write
void PerformCriticalSection(int *shm_shnum, char *filename)
{
	char msg[100];
	fprintf(stderr, "Child - %d: I am entering the critical section\n", process_num);
		
	sleep_time = rand() % 3;
	fprintf(stderr, "Child - %d: Going to sleep for %d seconds before incrementing...\n", process_num, sleep_time);
	sleep(sleep_time);
	// <<<<< entering critical section >>>>>
	(*shm_shnum)++; // increment the value in memory
	FormatTimeHMS(buffer, 26);
	sprintf(msg, "File modified by process number %d at time %s with shared num %d", process_num, buffer, *shm_shnum);
	FILE *fp;
	fp = fopen(filename, "a");
	fprintf(fp, "%s\n", msg);
	fclose(fp);
	// <<<<< leaving critical section >>>>>

	sleep_time = rand() % 3; // sleep for 0-2 seconds
	fprintf(stderr, "Child - %d: Going to sleep for %d seconds after incrementing...\n", process_num, sleep_time);
	sleep(sleep_time);

	fprintf(stderr, "Child - %d: I am leaving the critical section\n", process_num);
}

// waits for it's turn to enter the critical section, writes to a shared 
// memory variable, and kills itself
int main(int argc, char *argv[])
{
	if (LegalInt(argv[1]) == 0 || LegalInt(argv[2]) == 0 || LegalInt(argv[3]) == 0)
	{
		printf("%s\n", argv[1]);
	    fprintf(stderr, "ERROR: invalid args in slave\n");
		return -1;
	}
	
	srand(getpid()); // so that each process doesn't use the same seed and 
					// generate the same sequence of random numbers

	process_num = getpid();
	child_num = atoi(argv[1]);
	max_writes = atoi(argv[2]);
	num_slaves = atoi(argv[3]);
	
	// all of our shared memory variables
	int turn;
	int flag[num_slaves]; // 0=idle, 1=want_in, 2=in_cs
	int sharedNum;

	key_t key_turn = ftok("/tmp", 35);
	int shmid_turn = shmget(key_turn, sizeof(int), IPC_CREAT | 0666);
	int *shm_turn = shmat(shmid_turn, NULL, 0);
	turn = *shm_turn;
	
	key_t key_flag = ftok("/tmp", 93);
	int shmid_flag = shmget(key_flag, sizeof(int)*num_slaves, IPC_CREAT | 0666);
	int *shm_flag = shmat(shmid_flag, NULL, 0);
	for (i = 0; i < num_slaves; i++) flag[i] = *(shm_flag + i);

	key_t key_shnum = ftok("/tmp", 27);
	int shmid_shnum = shmget(key_shnum, sizeof(int), IPC_CREAT | 0666);
	int *shm_shnum = shmat(shmid_shnum, NULL, 0);
	sharedNum = *shm_shnum;
	
	do {
		do {
			*(shm_flag + child_num) = 1; // raise my flag
			j = *shm_turn;

			// wait until it's my turn
			while ( j != child_num )
				j = ( *(shm_flag + j) != 0 ) ? *shm_turn : (j + 1) % num_slaves;

			// declare intention to enter critical section
			*(shm_flag + child_num) = 2;

			// check that no one else is in critical section
			for ( j = 0; j < num_slaves; j++ )
			{
				if ( (j != child_num) && (*(shm_flag + j) == 2) )
					break;
			}
		} while ( (j < num_slaves) || ((*shm_turn != child_num) && (*(shm_flag + *shm_turn) != 0)) );

		// assign turn to self and enter critical section
		*shm_turn = child_num;
		PerformCriticalSection(shm_shnum, argv[4]);

		// exit section
		max_writes--;
		j = (*shm_turn + 1) % num_slaves;

		while ( *(shm_flag + j) == 0)
			j = (j + 1) % num_slaves;

		// assign turn to next waiting process and chang own flag to idle
		*shm_turn = j;
		*(shm_flag + child_num) = 0;
	}	while (max_writes > 0);

	fprintf(stderr, "Child - %d: I am going to die now. Goodbye.\n", process_num);
	exit(0);
}
