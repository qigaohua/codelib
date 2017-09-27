#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

/*int getopt_long(int argc, char * const argv[],
 *                const char *optstring,
 *                const struct option *longopts, int *longindex);
 **/

const char *optstr = "hvn:";

const struct option long_opt[] = {
	{"help",    0, NULL, 'h'},	
	{"vertion", 0, NULL, 'v'},
	{"name",    1, NULL, 'n'},
	{0,0,0,0}
};

int main(int argc, char **argv)
{
	int c;
	char *name;

	while ((c = getopt_long(argc, argv, optstr, long_opt, NULL)) != -1)
	{
		switch (c) 
		{
			case 'h':
				printf("help\n");
				break;

			case 'v':
				printf("vartion\n");
				break;

			case 'n':
				name = optarg;
				printf("name is %s\n", name);
				break;

			default:
			;	
		}
	}

	return 0;
}

