
#include <stdio.h>
#include <getopt.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int        opt;                                                        /* holds the current command line argument */
	const char *short_opt        = "hs:l:i:t:";                            /* valid short cmd args */
	struct     option long_opt[] = { { "help", no_argument, NULL, 'h' } }; /* valid long cmd args */
	int        max_slaves        = 5;                                      /* max number of slave processes */
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
				break;
			case 's':
				max_slaves = atoi(optarg);
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
}
