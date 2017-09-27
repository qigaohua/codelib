#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bin_search_tree.h"


bst_tree* bst_init()
{
    bst_tree *root;

    root = malloc(sizeof(bst_tree));
    if(root == NULL) {
        return NULL;
    }
    memset(root, 0, sizeof(bst_tree));
    return root;
}

/*二叉树节点个数*/
int GetNodeNum(bst_tree * root)  
{  
	if(root == NULL) // 递归出口  
		return 0;  
	return GetNodeNum(root->left) + GetNodeNum(root->right) + 1;  
}  

bst_tree* bst_insert(bst_tree **root, void* value, int (*node_cmp)(const void*, const void*))
{
	int ret, flag = 0;
    bst_tree  **node, *child;

    if(root == NULL) {
    	printf("root == null\n");
        return NULL;
    }

    node = root;
	while(*node != NULL) {
        /*待插入数据已经存在，则返回*/
    	ret = node_cmp((*node)->key, value);
        if(ret == 0) {
            //return *node;
            flag = 1;
            break;
        } else {
        	node = (ret < 0) ? &(*node)->left : &(*node)->right;
        }
    }

    child = (bst_tree *)malloc(sizeof(bst_tree));
    child->key = value;
    child->left = NULL;
    child->right = NULL;
    
    if(flag) {
        bst_tree* tmp = (*node)->left;
        (*node)->left = child;
        child->left = tmp;
    } else {
        *node = child;
    }

    return child;
}


bst_tree* bst_search(bst_tree *root, VALUE_TYPE value, int (*node_search)(const void*, VALUE_TYPE))
{
	int ret;
    bst_tree *p;
    p = root;
    if(p == NULL) {
    	//printf("p == null\n");
        return NULL;
    }
    ret = node_search(root->key, value);
    
    if(ret == 0)
    	return root;
    else if (ret < 0) // L
        return bst_search(p->left, value, node_search);
    else if (ret > 0) // R
        return bst_search(p->right, value, node_search);
    return NULL;
}


// int bst_delete(bst_tree *root, VALUE_TYPE value)
// {
	
// }

/*前序遍历*/
void bst_print(bst_tree *root, void (*key_print)(void* key))
{
	if(root)
    {
        //printf(" %d - %d", ((PSValueNode)root->key)->value1,((PSValueNode)root->key)->value2);
        key_print(root->key);
        if(root->left || root->right)
        {
            //printf("(");
            bst_print(root->left, key_print);
            if(root->right)
                //printf(",");
				bst_print(root->right, key_print);
            //printf(")");
        }
    }
}

void bst_destroy(bst_tree* root, void (*key_free)(void* key))
{
	if(root) {
        key_free(root->key);
        bst_destroy(root->left, key_free);
        bst_destroy(root->right, key_free);
    }
}


int ipv4_cmp(const void* key, const void* node)
{
	struct ipv4_addr* n1, *n2;

	n1 = (struct ipv4_addr*)key;
	n2 = (struct ipv4_addr*)node;
	
	if(n1->start < n2->start)
		return 1;
	else if(n1->start == n2->start){
		if(n1->end < n2->end)
			return 1;
		else if(n1->end == n2->end)
			return 0;
		else
			return -1;
	} else {
		return -1;
	}
	return 0;
}

int ipv4_search(const void* key, uint32_t value)
{
	struct ipv4_addr* n1;

    n1 = (struct ipv4_addr*)key;
    
    if(value >= n1->start && value <= n1->end) {
        return 0;
    } else if(value < n1->start) {
        return -1;
    } else {
        return 1;
    }

    return 0;
}

void ipv4_print(void* key)
{
	struct ipv4_addr* n;

	n = (struct ipv4_addr*) key;

	if(n) {
		printf("%u - %u t %d\n", n->start, n->end, n->type);
	}
}

void ipv4_free(void* key)
{
	if(key) {
		free(key);
	}
}


