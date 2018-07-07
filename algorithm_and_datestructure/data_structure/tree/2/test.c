#include <stdio.h>
#include <stdlib.h>
#include "bin_search_tree.h"



int main(int argc, char *argv[])  
{  
	int i, num;  
	bst_p root = NULL;  

	if (argc < 2) {  
		fprintf(stderr, "Usage: %s num\n", argv[0]);  
		exit(-1);  
	}  

	num = atoi(argv[1]);  
	value_type arr[num];  
	printf("Please enter %d integers:\n", num);  
	for (i = 0; i < num; i++) {  
		scanf("%d", &arr[i]);  
		bst_for_insert(&root, arr[i]);  
	}  

	printf("\ndelete before\n");  
	bst_for_printf(root, data_print);  
	printf("\n");  

	bst_for_delete(&root, 45);  

	printf("delete after\n");  
	bst_for_printf(root, data_print);  
	  
	return 0;  
}
