
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
void spawnSlaves(int num_slaves, int max_writes, char *filename)
{
	int i;
	pid_t pids[num_slaves];
	char arg1[20];	// holds the rank of the child (between 0 and num_slaves -1)
	char arg2[20];	// holds the number of times to write to file
	char arg3[20];	// holds the number of slaves that are being generated
	for (i = 0; i < num_slaves; i++)
	{
		if ((pids[i] = fork()) < 0)
		{
			perror("fork error");
			abort();
		}
		else if (pids[i] == 0)
		{
			snprintf(arg1, sizeof(arg1), "%d", i);
			snprintf(arg2, sizeof(arg2), "%d", max_writes);
			snprintf(arg3, sizeof(arg3), "%d", num_slaves);
			execl("./slave", "slave", arg1, arg2, arg3, filename, NULL);
			exit(0);
		}
	}	
}

int main(int argc, char *argv[])
{
	int			opt;                                                        /* holds the current command line argument */
	const char	*short_opt        = "hs:l:i:t:";                            /* valid short cmd args */
	struct		option long_opt[] = { { "help", no_argument, NULL, 'h' } }; /* valid long cmd args */
	int			num_slaves        = 5;                                      /* max number of slave processes */
	char		*filename         = "test.out";                             /* log file for output */
	int			num_increment     = 3;                                      /* how many times a slave should increment the value before terminating */
	int			kill_time         = 20;                                     /* amount of seconds before master will terminate itself */
	int			i;															/* iterator */

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
			// insert error checking...
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
				exit(1);
		}
	};

	// clear out the log file of any old content so we can properly append later
	FILE *fp;
	fp = fopen(filename, "w");
	fclose(fp);
	
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

	spawnSlaves(num_slaves, num_increment, filename);

	int status;
	pid_t pid;
	while (num_slaves > 0)
	{
		pid = wait(&status);
		fprintf(stderr, "Child with PID %ld has died :(\n", (long)pid, status);
		--num_slaves;
	}
	
	// cleanup
	fprintf(stderr, "Master - I am clearing memory now\n");
	
	shmdt(shm_shnum);
	shmdt(shm_turn);
	shmdt(shm_flag);
	
	shmctl(shmid_shnum, IPC_RMID, NULL);
	shmctl(shmid_turn, IPC_RMID, NULL);
	shmctl(shmid_flag, IPC_RMID, NULL);

	exit(0);
}

