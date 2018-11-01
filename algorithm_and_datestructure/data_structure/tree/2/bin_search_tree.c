#include <stdio.h>
#include <stdlib.h>
#include "bin_search_tree.h"



bst_p bst_for_search(bst_p *root, value_type data)
{
	bst_p s;

	if (NULL == *root) {
		printf("file:%s line:%d bin_search_tree error\n", __FILE__, __LINE__);
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
static bst_p bst_for_cmp(bst_p *root, value_type data)
{
	bst_p s, p;

	if (NULL == *root) {
		printf("file:%s line:%d bin_search_tree error\n", __FILE__, __LINE__);
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

int bst_for_insert(bst_p *root, value_type data)
{
	bst_p child, p;

	child = malloc(sizeof(bst_t));
	if (NULL == child) {
		perror("molloc error");
		exit(-1);
	}

	child->data = data;
	child->lchild = NULL;
	child->rchild = NULL;

	if (NULL == *root) 
		*root = child;
	else {
		p = bst_for_cmp(root, data);
		if (NULL == p) {
			printf("data already exists or error\n");
			free(child);
			return -1;
		}

		if (data > p->data)
			p->rchild = child;
		else
			p->lchild = child;
	}

	return 0;
}

void data_print(void* data)
{
	int *value = (int*)data;
	printf("%d\n", *value);	
}

void bst_for_printf(bst_p root, void (*data_printf)(void* data))
{
	if (NULL != root) {
		data_printf(&root->data);
		bst_for_printf(root->lchild, data_printf);
		bst_for_printf(root->rchild, data_printf);
	}
}


int bst_for_delete(bst_p *root, value_type data)
{
	bst_p p = *root, s, parent;

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
}


