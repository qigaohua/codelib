#ifndef __BIN_SEARCH_TREE__
#define __BIN_SEARCH_TREE__

typedef unsigned int value_type;

typedef struct _bin_search_tree {
	value_type data;
	struct _bin_search_tree *lchild, *rchild;
} bst_t, *bst_p;



void data_print(void* data);
bst_p bst_for_search(bst_p *root, value_type data);
static bst_p bst_for_cmp(bst_p *root, value_type data);
int bst_for_insert(bst_p *root, value_type data);
void bst_for_printf(bst_p root, void (*data_print)(void* data));
int bst_for_delete(bst_p *root, value_type data);




#endif

