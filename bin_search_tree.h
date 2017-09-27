#ifndef __JFIND_BIN_SEARCH_TREE_H__
#define __JFIND_BIN_SEARCH_TREE_H__

typedef unsigned int VALUE_TYPE;

struct bin_search_tree
{
    //datatype key;
    char* key;
    struct bin_search_tree *left,*right;
};

typedef struct bin_search_tree bst_tree;


/**
 * insert node in bin_tree
 * 
 */
bst_tree* bst_insert(bst_tree **root, VALUE_TYPE value, int (*node_cmp)(const void*, const void*));

/*查找，找到返回1，否则，返回0*/
bst_tree* bst_search(bst_tree *root, VALUE_TYPE value, int (*node_search)(const void*, VALUE_TYPE));

/*删除节点值为value的节点，成功返回1，否则，返回0*/
//int bst_delete(bst_tree *root, SValueNode value);

/*中序输出bst树*/
void bst_print(bst_tree *root, void (*key_print)(void* key));
void bst_destroy(bst_tree* root, void (*key_free)(void* key));










struct ipv4_addr {
	uint32_t start;
	uint32_t end;
	int type;		//0-static 1-dynamic 2-reserve
};

typedef bst_tree  ipv4_pool;
/**
 * [ipv4_cmp description]
 * @param  key  [node in tree]
 * @param  node [compare node]
 * @return      [key:node 1 <; 0 =; -1 >]
 */
int ipv4_cmp(const void* key, const void* node);


/**
 * [ipv4_search description]
 * @param  key   [node in tree]
 * @param  value [compare value]
 * @return       [key:value 1 <; 0 =; -1 >]
 */
int ipv4_search(const void* key, uint32_t value);

/**
 * [ipv4_print description]
 * @param key [node in tree]
 */
void ipv4_print(void* key);

void ipv4_free(void* key);

/**
 * 
 */

#define ADD_IPv4_INFO(tree, addr) \
	(bst_insert(tree, addr, ipv4_cmp) == NULL ? -1 : 0)

#define SEARCH_IPv4_INFO(tree, addr) \
	(bst_search(tree, addr, ipv4_search) == NULL ? -1 : 0)

#define SEARCH_IPv4_NODE(tree, addr) \
	bst_search(tree, addr, ipv4_search)


#define PRINT_IPv4_POOL(tree) \
	bst_print(tree, ipv4_print)

#define FREE_IPv4_POOL(tree) \
	bst_destroy(tree, ipv4_free)
#endif //__JFIND_BIN_SEARCH_TREE_H__
