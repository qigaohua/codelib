#ifndef __BIN_SEARCH_TREE__
#define __BIN_SEARCH_TREE__


#define False 0
#define True (!(False))

#define rbnode_is_red(node)    ((node)->color == RED)
#define rbnode_is_block(node)  ((node)->color == BLACK)
#define rbnode_set_block(node) ((node)->color = BLACK)
#define rbnode_set_red(node)   ((node)->color = RED)

#define rbnode_parent(node) ((node)->parent)


typedef enum {
    RED = 0,
    BLACK = 1,
} Color;

typedef int BOOL;

typedef unsigned int Type;
typedef void (*value_handle)(void *);


typedef struct _bin_search_tree bst_t, *bst_pt;
struct _bin_search_tree {
	Type data;
    Color color; 
	bst_t  *lchild;
    bst_t  *rchild;
    bst_t  *parent;
} ;



// void data_print(void* data);
bst_pt bst_for_search(bst_pt *root, Type data);
static bst_pt bst_for_cmp(bst_pt *root, Type data);
int bst_for_insert(bst_pt *root, Type data);
void bst_for_printf(bst_pt root);
int bst_for_delete(bst_pt *root, Type data);




#endif

