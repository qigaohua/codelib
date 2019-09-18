#include <stdio.h>
#include <stdlib.h>

#include "gh_rbtree.h"

#ifdef _DEBUG_
#define rbtree_print(fd, fmt, ...) \
    fprintf(fd, "%s:%d " fmt "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define rbtree_print(fd, fmt, ...) 
#endif

static value_handle value_print = NULL;


void bst_set_print (value_handle func)
{
    if (!value_print)
        value_print = func;
}


bst_pt bst_for_search(bst_pt *root, Type data)
{
	bst_pt s;

	if (NULL == *root) {
		rbtree_print(stderr, "Ivalied param.");
		return NULL;
	}
	
	s = *root;
	while (NULL != s) {
		if (s->data == data)
			return NULL;
		else 
			s = (s->data > data) ? (s->lchild) : (s-> rchild);
	}
	
	return s;
}


/*for insert*/
static bst_pt bst_for_cmp(bst_pt *root, Type data)
{
	bst_pt s, p;

	if (NULL == *root) {
		rbtree_print(stderr, "Ivalied param.");
		return NULL;
	}
	
	s = *root;
	while (NULL != s) {
		p = s;
		if (s->data == data)
			return NULL;
		else 
			s = (s->data > data) ? (s->lchild) : (s-> rchild);
	}
	
	return p;
}


bst_pt new_node (Type d, Color color)
{
    bst_pt new_node = (bst_pt) malloc (sizeof(bst_t));
    if (NULL == new_node) {
		rbtree_print(stderr, "molloc failed.");
        return NULL;
    }

    new_node->data = d;
    new_node->color = color;
    new_node->lchild = NULL;
    new_node->rchild = NULL;
    new_node->parent = NULL;

    return new_node;
}


/* 
 * 对红黑树的节点(x)进行左旋转
 *
 * 左旋示意图(对节点x进行左旋)：
 *      px                              px
 *     /                               /
 *    x                               y                
 *   /  \      --(左旋)-->           / \                #
 *  lx   y                          x  ry     
 *     /   \                       /  \
 *    ly   ry                     lx  ly  
 *
 *
 */
static void rbtree_left_rotate(bst_pt *root, bst_pt x)
{
    // 设置x的右孩子为y
    bst_pt y = x->rchild;

    rbtree_print(stdout, "");
    // 将 “y的左孩子” 设为 “x的右孩子”；
    // 如果y的左孩子非空，将 “x” 设为 “y的左孩子的父亲”
    x->rchild = y->lchild;
    rbtree_print(stdout, "");
    if (y->lchild != NULL)
        y->lchild->parent = x;

    rbtree_print(stdout, "");
    // 将 “x的父亲” 设为 “y的父亲”
    y->parent = x->parent;

    if (x->parent == NULL)
    {
        //tree = y;            // 如果 “x的父亲” 是空节点，则将y设为根节点
        *root = y;            // 如果 “x的父亲” 是空节点，则将y设为根节点
    }
    else
    {
        if (x->parent->lchild == x)
            x->parent->lchild = y;    // 如果 x是它父节点的左孩子，则将y设为“x的父节点的左孩子”
        else
            x->parent->rchild = y;    // 如果 x是它父节点的左孩子，则将y设为“x的父节点的左孩子”
    }
    rbtree_print(stdout, "");
    
    // 将 “x” 设为 “y的左孩子”
    y->lchild = x;
    rbtree_print(stdout, "");
    // 将 “x的父节点” 设为 “y”
    x->parent = y;
}

/* 
 * 对红黑树的节点(y)进行右旋转
 *
 * 右旋示意图(对节点y进行左旋)：
 *            py                               py
 *           /                                /
 *          y                                x                  
 *         /  \      --(右旋)-->            /  \                     #
 *        x   ry                           lx   y  
 *       / \                                   / \                   #
 *      lx  rx                                rx  ry
 * 
 */
static void rbtree_right_rotate(bst_pt *root, bst_pt y)
{
    // 设置x是当前节点的左孩子。
    bst_pt x = y->lchild;

    // 将 “x的右孩子” 设为 “y的左孩子”；
    // 如果"x的右孩子"不为空的话，将 “y” 设为 “x的右孩子的父亲”
    rbtree_print(stdout, "");
    y->lchild = x->rchild;
    rbtree_print(stdout, "");
    if (x->rchild != NULL)
        x->rchild->parent = y;

    // 将 “y的父亲” 设为 “x的父亲”
    x->parent = y->parent;
    rbtree_print(stdout, "");

    if (y->parent == NULL) 
    {
        //tree = x;            // 如果 “y的父亲” 是空节点，则将x设为根节点
        *root = x;            // 如果 “y的父亲” 是空节点，则将x设为根节点
    }
    else
    {
        if (y == y->parent->rchild)
            y->parent->rchild = x;    // 如果 y是它父节点的右孩子，则将x设为“y的父节点的右孩子”
        else
            y->parent->lchild = x;    // (y是它父节点的左孩子) 将x设为“x的父节点的左孩子”
    }
    rbtree_print(stdout, "");

    // 将 “y” 设为 “x的右孩子”
    x->rchild = y;

    // 将 “y的父节点” 设为 “x”
    y->parent = x;
}


int rbtree_insert_fixup(bst_pt *root, bst_pt node)
{
    /* case 1: 插入的节点是根节点 */
    if (*root == node) {
        node->color = BLACK;
        return 0;
    }

    /* case 2: 插入的节点的父节点是黑色 */
    if (rbnode_is_block(node->parent))
        return 0;

    /* case 3: 插入的节点的父节点是红色 */
    bst_pt parent ;
    while ((parent = rbnode_parent(node)) && rbnode_is_red(parent)) {
        bst_pt gparent = rbnode_parent(parent);
        bst_pt uncle;

        /* case 3.1 叔叔节点是祖父节点的右节点 */
        if (gparent->lchild == parent) {
            uncle = gparent->rchild;
            /* case 3.1.1 叔叔节点是红色 */
            if (uncle && rbnode_is_red(uncle)) {
                rbnode_set_block(parent);
                rbnode_set_block(uncle);
                rbnode_set_red(gparent);
                node = gparent;
                continue;
            }
            else {
                /* case 3.1.2 叔叔节点是黑色,且当前节点是其父节点的右孩子 */
                if (parent->rchild == node) {
                    bst_pt tmp = parent;
                    parent = node;
                    node = tmp;
                    rbtree_left_rotate(root, node);
                }

                /* case 3.1.3 叔叔节点是黑色,且当前节点是其父节点的左孩子 */
                rbnode_set_block(parent);
                rbnode_set_red(gparent);
                rbtree_right_rotate(root, gparent);
            }

        } 
        /* case 3.2 叔叔节点是祖父节点的左节点 */
        else {
            uncle = gparent->lchild;

            /* case 3.2.1 叔叔节点是红色 */
            if (uncle && rbnode_is_red(uncle)) {
                rbnode_set_block(parent);
                rbnode_set_block(uncle);
                rbnode_set_red(gparent);
                node = gparent;
                continue;
            }
            else {
                /* case 3.2.2 叔叔节点是黑色,且当前节点是其父节点的右孩子 */
                if (parent->lchild == node) {
                    bst_pt tmp = parent;
                    parent = node;
                    node = tmp;
                    rbtree_right_rotate(root, node);
                }

                /* case 3.2.3 叔叔节点是黑色,且当前节点是其父节点的左孩子 */
                rbnode_set_block(parent);
                rbnode_set_red(gparent);
                rbtree_left_rotate(root, gparent);
            }
        }
    }

    rbnode_set_block(*root);

    return 0;
}


int bst_for_insert(bst_pt *root, Type data)
{
	bst_pt child, p;


    child = new_node(data, BLACK);

	if (NULL == *root) 
		*root = child;
    else {
        p = bst_for_cmp(root, data);
        if (NULL == p) {
            rbtree_print(stderr, "data already exist or error");
            free(child);
            return -1;
        }
        child->parent = p;

		if (data > p->data)
			p->rchild = child;
		else
			p->lchild = child;

        child->color = RED;
	}

 
    rbtree_insert_fixup(root, child);

	return 0;
}


void data_print(bst_pt node)
{
    if (node->parent)
        printf("%s : %d   [父节点: %s : %d]\n", node->color == RED ? "red" : "block", node->data, 
                node->parent->color == RED ? "red" : "block", node->parent->data);	
    else 
        printf("%s : %d   [根节点]\n", node->color == RED ? "red" : "block", node->data);	

}


void bst_for_printf(bst_pt root)
{
	if (NULL != root) {
        if (value_print)
            value_print(&root->data);
        else
		    data_print(root);
		bst_for_printf(root->lchild);
		bst_for_printf(root->rchild);
	}
}


bst_pt rbtree_search (bst_pt *root, Type data) 
{
    if (*root == NULL) {
        rbtree_print(stderr, "Invalid param.");
        return NULL;
    }

    if ((*root)->data == data)
        return *root;
    else if ((*root)->data > data) 
        return rbtree_search(&(*root)->lchild, data);
    else
        return rbtree_search(&(*root)->rchild, data);
}


bst_pt rbtree_max_node(bst_pt node)
{
    if (node == NULL) {
        rbtree_print(stderr, "Invalid param.");
        return NULL;
    }

    bst_pt max_node = node;
    while (max_node->rchild) 
        max_node = max_node->rchild;

    return max_node;
}


bst_pt rbtree_min_node(bst_pt node)
{
    if (node == NULL) {
        rbtree_print(stderr, "Invalid param.");
        return NULL;
    }

    bst_pt min_node = node;
    while (min_node->lchild) 
        min_node = min_node->lchild;

    return min_node;
}


/*
 * @brief rbtree_successor 查找某个节点的后继节点(即查找"树中数据值大于该结点"的"最小结点")
 *
 * @param node 树中某个节点
 *
 * @return  
 */
bst_pt rbtree_successor(bst_pt node)
{
    if (node == NULL) {
        rbtree_print(stderr, "Invalid param.");
        return NULL;
    }

    bst_pt parent;

    /* case 1: 该节点有右孩子 */
    if (node->rchild)
        return rbtree_min_node(node->rchild);

    parent = node->parent;
    /* case 2: 没有右孩子，且该节点是其父节点的左孩子 */
    if (parent->lchild == node)
        return parent;

    /* case 3: 没有右孩子，且该节点是其父节点的右孩子 */
    while (parent != NULL && node == parent->rchild) {
        node = parent;
        parent = parent->parent;
    }

    return parent;
}


/*
 * @brief rbtree_predecessor 查找某个节点的前驱节点(即查找"树中数据值小于该结点"的"最大结点")
 *
 * @param node 树中某个节点
 *
 * @return  
 */
bst_pt rbtree_predecessor(bst_pt node) 
{
    if (node == NULL) {
        rbtree_print(stderr, "Invalid param.");
        return NULL;
    }

    bst_pt parent;

    /* case 1: 该节点有左孩子 */
    if (node->lchild)
        return rbtree_max_node(node->lchild);

    parent = node->parent;
    /* case 2: 没有左孩子，且该节点是其父节点的右孩子 */
    if (parent->rchild == node)
        return parent;

    /* case 3: 没有左孩子，且该节点是其父节点的左孩子 */
    while (parent != NULL && node == parent->lchild) {
        node = parent;
        parent = parent->parent;
    }

    return parent;

}


int rbtree_delete_fixup(bst_pt *root, bst_pt node, bst_pt parent) 
{
    bst_pt brother = NULL;

    if (*root == NULL) {
        rbtree_print(stderr, "Invalid param.");
        return -1;
    }

    while ((!node || rbnode_is_block(node)) && node != *root) {
        if (parent->lchild == node) {
            brother = parent->rchild;
            /* case 1: 兄弟节点是红色 */
            if (rbnode_is_red(brother)) {
                rbnode_set_block(brother);
                rbnode_set_red(parent);
                rbtree_left_rotate(root, parent);
                brother = parent->rchild;
            }
            /* Case 2: 兄弟是黑色，且俩个孩子也都是黑色的 */
            if ((!brother->lchild || rbnode_is_block(brother->lchild)) && 
                    (!brother->lchild || rbnode_is_block(brother->rchild))) {
                rbnode_set_red(brother);
                node = parent;
            } 
            else {
                /* Case 3: 兄弟是黑色的，并且左孩子是红色，右孩子为黑色  */
                if (!brother->rchild || rbnode_is_block(brother->rchild)) {
                    rbnode_set_block(brother->lchild);
                    rbnode_set_red(brother);
                    rbtree_right_rotate(root, brother);
                    brother = parent->rchild;

                }
                /* Case 4: 兄弟是黑色的；并且右孩子是红色的，左孩子任意颜色 */
                brother->color = parent->color;
                rbnode_set_block(parent);
                rbnode_set_block(brother->rchild);
                rbtree_left_rotate(root, parent);
                node = *root;
                break;
            }
        }
        else {
            brother = parent->lchild;
            /* case 1: 兄弟节点是红色 */
            if (rbnode_is_red(brother)) {
                rbnode_set_block(brother);
                rbnode_set_red(parent);
                rbtree_right_rotate(root, parent);
                brother = parent->lchild;
            }
            /* Case 2: 兄弟是黑色，且俩个孩子也都是黑色的 */
            if ((!brother->lchild || rbnode_is_block(brother->lchild)) && 
                    (!brother->lchild || rbnode_is_block(brother->rchild))) {
                rbnode_set_red(brother);
                node = parent;
            } 
            else {
                /* Case 3: 兄弟是黑色的，并且左孩子是红色，右孩子为黑色  */
                if (!brother->lchild || rbnode_is_block(brother->lchild)) {
                    rbnode_set_block(brother->rchild);
                    rbnode_set_red(brother);
                    rbtree_left_rotate(root, brother);
                    brother = parent->lchild;

                }
                /* Case 4: 兄弟是黑色的；并且右孩子是红色的，左孩子任意颜色 */
                brother->color = parent->color;
                rbnode_set_block(parent);
                rbnode_set_block(brother->lchild);
                rbtree_right_rotate(root, parent);
                node = *root;
            }

        }
    }

    if (node)
        rbnode_set_block(node);

    return 0;
}


int bst_for_delete(bst_pt *root, Type data)
{
    bst_pt d_node = NULL;

    if (*root == NULL) {
        rbtree_print(stderr, "Invalid param.");
        return -1;
    }

    d_node = rbtree_search(root, data);
    if (d_node == NULL) {
        rbtree_print(stdout, "The node not exist.");
        return 0;
    }

    bst_pt child = NULL, parent = NULL;
    Color color;

    if (d_node->lchild && d_node->rchild) {
        bst_pt replace = NULL;

        /* 寻找替代删除节点位置的节点replace */
        replace = rbtree_successor(d_node);

        /* 如果删除的节点不是根节点, 即存在父节点 */
        if (rbnode_parent(d_node)) {
            if (rbnode_parent(d_node)->lchild == d_node)
                rbnode_parent(d_node)->lchild = replace;
            else 
                rbnode_parent(d_node)->rchild = replace;
        } 
        else 
            *root = replace;

        color = replace->color;
        child = replace->rchild;

        parent = rbnode_parent(replace);

        if (parent == d_node) {
            parent = replace; 
        } 
        else {
            if (child)
                child->parent = parent;
            parent->lchild = child;

            replace->rchild = d_node->rchild;
            d_node->rchild->parent = replace;
        }

        replace->parent = d_node->parent;
        replace->color = d_node->color;
        replace->lchild = d_node->lchild;
        d_node->lchild->parent = replace;

        if (color == BLACK)
            rbtree_delete_fixup(root, child, parent)
            ;
        free(d_node);

        return 0;
    }

    child = d_node->lchild ? d_node->lchild : d_node->rchild;
    parent = d_node->parent;

    if (child)
        child->parent = parent;

    color = d_node->color;

    if (parent) {
        if (parent->lchild == d_node)
            parent->lchild = child;
        else 
            parent->rchild = child; 
    }
    else
        *root = child;
        
    if (color == BLACK) 
        rbtree_delete_fixup(root, child, parent)
            ;

    free (d_node);
        
	return 0;




#if 0
	bst_pt p = *root, s, parent;

	if (NULL == p) {
		// printf("bst is null\n");
		return 0;
	}

	if (p->data == data) {
		if ((NULL == p->lchild) && (NULL == p->rchild)) {
			*root = NULL;
		} else if ((NULL != p->lchild) && (NULL == p->rchild)) {
			*root = p->lchild;
		} else if ((NULL == p->lchild) && (NULL != p->rchild)) {
			*root = p->rchild;
		} else {
			s = p->rchild;
			if (NULL == s->lchild)
				s->lchild = p->lchild;
			else {
				while (NULL != s->lchild) {
					parent = s;
					s = s->lchild;
				}
				parent->lchild = s->rchild;
				s->lchild = p->lchild;
				s->rchild = p->rchild;
			}
            *root = s;
        }
        free(p);

    } else if (p->data > data) {
		bst_for_delete(&(p->lchild), data);
	} else {
		bst_for_delete(&(p->rchild), data);
	}

	return 0;
#endif
}


