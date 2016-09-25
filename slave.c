#include <stdio.h>
#include <ctype.h>

int process_num = 0;

int LegalInt(char *str)
{
	while (*str)
		if (!isdigit(*str++)) return 0;
	return 1;
}

int main(int argc, char *argv[])
{
	if (argc !=  2)
	{
		fprintf(stderr, "ERROR: please only pass in a single process number\n");
		return -1;
	}
	else if (LegalInt(argv[1]) == 0)
	{
		printf("%s\n", argv[1]);
		fprintf(stderr, "ERROR: please pass a valid integer\n");
		return -1;
	}
	else
	{
		process_num = atoi(argv[1]);
		printf("process number: %d\n", process_num);
		return 1;
	}

}
