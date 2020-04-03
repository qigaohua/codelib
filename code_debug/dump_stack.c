#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <execinfo.h>

#define BACKTRACE_SIZE 256


void rte_dump_stack(void)
{
	void *func[BACKTRACE_SIZE];
	char **symb = NULL;
	int size;

	size = backtrace(func, BACKTRACE_SIZE);
	symb = backtrace_symbols(func, size);

	if (symb == NULL)
		return;

	while (size > 0) {
		fprintf(stderr, 
			"%d: [%s]\n", size, symb[size - 1]);
		size --;
	}

	free(symb);
}

