
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/* create num_slaves slave processes that will write to a file max_writes 
 * times*/
void spawnSlaves(int num_slaves, int max_writes)
{
	int i;
	pid_t pids[num_slaves];
	char arg1[20];	// holds the process number
	char arg2[20];	// holds the number of times to write to file

	for (i = 0; i < num_slaves; i++)
	{
		if ((pids[i] = fork()) < 0)
		{
			perror("fork error");
			abort();
		}
		else if (pids[i] == 0)
		{
			snprintf(arg1, sizeof(arg1), "%d", getpid());
			snprintf(arg2, sizeof(arg2), "%d", max_writes);
			execl("./slave", "slave", arg1, arg2, NULL);
			exit(0);
		}
	}	
}

CreateSharedMemory()
{
	
}

int main(int argc, char *argv[])
{
	int        opt;                                                        /* holds the current command line argument */
	const char *short_opt        = "hs:l:i:t:";                            /* valid short cmd args */
	struct     option long_opt[] = { { "help", no_argument, NULL, 'h' } }; /* valid long cmd args */
	int        num_slaves        = 5;                                      /* max number of slave processes */
	char       *filename         = "test.out";                             /* log file for output */
	int        num_increment     = 3;                                      /* how many times a slave should increment the value before terminating */
	int        kill_time         = 20;                                     /* amount of seconds before master will terminate itself */
	

	/* process cmd args */
	while ((opt = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1)
	{
		switch (opt)
		{
			case -1:
			case 0:
				break;
			case 'h':
				printf("Usage: %s [OPTIONS]\n", argv[0]);
				printf("    -h, --help    prints this help message\n");
				printf("    -s [x]        maximum number of slave processes (default: 5)\n");
				printf("    -l [filename] specify the log file to use (default: test.out)\n");
				printf("    -i [y]        specify how many times each slave should increment and write to the file before terminating (default: 3)\n");
				printf("    -t [z]        the time in seconds when the master will terminate itself (default: 20)\n");
				exit(0);
				break;
			case 's':
				num_slaves = atoi(optarg);
				break;
			case 'l':
				filename = optarg;
				break;
			case 'i':
				num_increment = atoi(optarg);
				break;
			case 't':
				kill_time = atoi(optarg);
				break;
			default:
				printf("%s: invalid option -- %c\n", argv[0], opt);
				printf("Try '%s --help' for more information.\n", argv[0]);
				return -2;
		}
	};
	
	int shmid;
	key_t key;
	int shm_val = 0;
	int *shm = &shm_val;

	// name the shared memory segment
	//if ((key = ftok("/tmp", 199)) == -1)
	//{
	//	perror("ftok");
	//	exit(1);
	//}
	key = 5678;

	// create the segment
	if ((shmid = shmget(key, 27, IPC_CREAT | 0666)) < 0)
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

	spawnSlaves(num_slaves, num_increment);

	int status;
	pid_t pid;
	while (num_slaves > 0)
	{
		pid = wait(&status);
		//printf("Child with PID %ld has died :(\n", (long)pid, status);
		--num_slaves;
	}
	
	// cleanup
	(*shm) = 0;
	shmdt(shm);
	shmctl(shmid, IPC_RMID, NULL);
	exit(0);
}
