#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dump.h"



int main()
{
	char *s = "abcdefg";

	dump(stdout, s, strlen(s));

	return 0;
}
