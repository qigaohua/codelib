#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gh_list.h"


void gh_list_dump (const char *what, gh_list_t *gl)
{
		int i;
		printf ("%s: #%p\t cnt %d\t size %d\t elems %p\n", 
						what, gl, gl->gh_cnt, gl->gh_size, gl->elems);
		for (i = 0; i < gl->gh_cnt; i ++) {
				printf ("  #%d: %p at &%p\n", i, gl->elems[i], &gl->elems[i]);
		}
}


int gh_list_grow (gh_list_t* gl, int32_t size)
{
		if (gl->gh_flags & GH_LIST_F_NOTGROWS)
				return -1;
		gl->gh_size += size;
		if (unlikely(gl->gh_size == 0))
				return 0;
		gl->elems =	(void **)gh_realloc(gl->elems, 
						sizeof(*gl->elems) * gl->gh_size);

		return 0;
}


gh_list_t* gh_list_init (gh_list_t *gl, int32_t size, func cb_free)
{
		check_param(gl, NULL);

		memset(gl, 0, sizeof(*gl));
		if (size > 0)
				gh_list_grow(gl, size);

		gl->cb_free = cb_free;

		return gl;
}

gh_list_t* gh_list_new (int32_t size, func cb_free)
{
		gh_list_t *gl = NULL;

		gl = (gh_list_t *)gh_malloc(sizeof(*gl));

		gh_list_init(gl, size, cb_free);
		gl->gh_flags |= GH_LIST_F_ALLOCATED;

		return gl;
}

static __thread 
int (*gh_list_cmp_curr) (const void *, const void *);

static __inline
int gh_list_cmp_trampoline (const void *_a, const void *_b) {
		const void *a = *(const void **)_a, *b = *(const void **)_b;

		return gh_list_cmp_curr(a, b);
}

int32_t gh_list_sort (gh_list_t *gl, 
				int32_t (*cmp)(const void *_a, const void *_b))
{
		check_param(gl, -1);
		check_param(cmp, -1);

		gh_list_cmp_curr = cmp;
		qsort(gl->elems, gl->gh_cnt, 
						sizeof(*gl->elems), gh_list_cmp_trampoline);
		gl->gh_flags |= GH_LIST_F_SORTED;

		return 0;
}

void*  gh_list_elem (gh_list_t *gl, int32_t i)
{
		if (likely(gl->gh_cnt > i)) 
				return (void *)gl->elems[i];
		return NULL;
}


void* gh_list_find (gh_list_t *gl, const void *elem,
				int32_t (*cmp)(const void *_a, const void *_b))
{
		int i;
		void *e;

		check_param(gl, NULL);
		check_param(elem, NULL);
		check_param(cmp, NULL);

		if (gl->gh_flags & GH_LIST_F_SORTED) {
				void **g;

				gh_list_cmp_curr = cmp;
				g = (void **)bsearch(&elem, gl->elems, gl->gh_cnt, 
								sizeof(*gl->elems), gh_list_cmp_trampoline);

				return g ? *g : NULL;
		}

		GH_LIST_FOREACH (gl, i, e) {
				if (!cmp (elem, e))
						return e;
		}

		return NULL;
}


void gh_list_free_cb (gh_list_t *gl, void *ptr)
{
		if (gl->cb_free && ptr)
				gl->cb_free(ptr);
}


int32_t gh_list_add_noduplicate (gh_list_t *gl, void *elem, 
				int32_t (*cmp)(const void *_a, const void *_b))
{
		check_param((gl && elem && cmp), -1);

		if (gh_list_find(gl, elem, cmp)) 
				return 0;

		if (gl->gh_cnt + 1 >= gl->gh_size) 
				gh_list_grow(gl, gl->gh_size ? gl->gh_size * 2 : 16);

		gl->elems[gl->gh_cnt ++] = elem;
		gl->gh_flags &= ~GH_LIST_F_SORTED;

		return 0;
}


int32_t gh_list_add (gh_list_t *gl, void *elem)
{
		check_param((gl || elem), -1);

		if (gl->gh_flags & GH_LIST_F_UNIQUE) 
				printf ("warning: the list not allow duplicates," 
								"you should use gh_list_add_noduplicate\n");

		if (gl->gh_cnt + 1 >= gl->gh_size) 
				gh_list_grow(gl, gl->gh_size ? gl->gh_size * 2 : 16);

		gl->elems[gl->gh_cnt ++] = elem;
		gl->gh_flags &= ~GH_LIST_F_UNIQUE;
		gl->gh_flags &= ~GH_LIST_F_SORTED;

		return 0;
}


void* gh_list_remove_index(gh_list_t *gl, int32_t idx)
{
		void *e = NULL;

		if (likely(gl->gh_cnt > (idx+1))) {
				e = gl->elems[idx];
				memmove(&gl->elems[idx], 
								&gl->elems[idx + 1],
								sizeof(*gl->elems) * (gl->gh_cnt - (idx+1)));
		}
		gl->gh_cnt --;

		return e;
}

void* gh_list_remove (gh_list_t *gl, void *elem)
{
		int32_t i;
		void *e;

		check_param(gl, NULL);
		check_param(elem, NULL);

		GH_LIST_FOREACH(gl, i, e) {
				if (e == elem) {
						gh_list_remove_index(gl, i);
						return e;
				}
		}

		return NULL;
}

void* gh_list_remove_cmp (gh_list_t *gl, void *elem, 
				int32_t (*cmp)(void *_a, void *_b))
{
		int32_t i;
		void *e;
		// GH_UNUSED(gl);
		check_param(gl, NULL);
		check_param(elem, NULL);
		check_param(cmp, NULL);

		GH_LIST_FOREACH(gl, i, e) {
				if (e == elem || !cmp(e, elem)) {
						gh_list_remove_index(gl, i);
						return e;
				}
		}

		return NULL;
}


int32_t gh_list_multi_remove_cmp (gh_list_t *gl, void *elem, 
				int32_t (*cmp)(const void *_a, const void *_b))
{
		int32_t i;
		void *e;
		int32_t cnt = 0;
		// GH_UNUSED(gl);
		check_param(gl, -1);
		check_param(elem, -1);
		check_param(cmp, -1);

		GH_LIST_FOREACH(gl, i, e) {
				if (e == elem || !cmp(e, elem)) {
						gh_list_remove_index(gl, i);
						gh_list_free_cb(gl, e);
						cnt++;
				}
		}

		return cnt;
}


/*
 * 因为 gl->elems 是 (void **)（指针的指针）, 那么
 * 在qsort函数中返回比较的元素是 (void **)， 
 * 所以我们需要一个跳板函数将 (void **) -> (void *) 
 */

void __inline gh_list_clear (gh_list_t *gl)
{
		gl->gh_cnt = 0;
}


int32_t __inline gh_list_cnt (gh_list_t *gl)
{
		return gl->gh_cnt;
}

int32_t __inline gh_list_empty (gh_list_t *gl)
{
		return gl->gh_cnt == 0;
}


/* 默认copy 指针 */
void* gh_list_onlycopy_ptr (void *s, void *mt)
{
		GH_UNUSED(mt)
				return (void *)s;
}


int32_t gh_list_copy_to (gh_list_t *dst, gh_list_t *src,
				void* (*copy)(void* _d, void *_s),
				void* match)
{
		int i;
		void *s, *d = NULL;

		check_param(dst, -1);
		check_param(src, -1);

		if (!copy)
				copy = gh_list_onlycopy_ptr;

		GH_LIST_FOREACH (src, i, s) {
				if (!s) 
						break;
				d = copy(s, match);
				if (d)
						gh_list_add(dst, d);
		}
		dst->cb_free = src->cb_free;
		dst->gh_flags = src->gh_flags;

		return 0;
}

void gh_list_free (gh_list_t *gl)
{
		int i;

		if (gl) {
				for (i = 0; i < gl->gh_cnt; i ++) {
						if (gl->cb_free) 
								gl->cb_free(gl->elems[i]);
						else
								free(gl->elems[i]);

				}
				free(gl->elems);

				if (gl->gh_flags & GH_LIST_F_ALLOCATED) 
						free(gl);
		}
}


gh_list_t* gh_list_copy (gh_list_t *src,
				void* (*copy)(void* _d, void *_s),
				void* match)
{
		gh_list_t *dst = NULL;

		check_param(src, NULL);

		dst = gh_list_new(src->gh_size, src->cb_free);

		if (gh_list_copy_to(dst, src, copy, match) != 0) {
				gh_list_free(dst);
				dst = NULL;
		}

		return dst;
}


#define TEST
#ifdef TEST

struct st {
		int a;
		double b;
		char *c;
};

int find_num = 0;
void test_free(void *ptr) {
		struct st *s = (struct st *)ptr;
		if (s) {
				if (s->c)
						free(s->c);
		}
		free(s);
}

int32_t test_cmp(void *_a, void *_b) {
		struct st *sa = (struct st *)_a;
		struct st *sb = (struct st *)_b;
		if (sa && sb) {
				return !(sa->a == sb->a);
		}
		return 1;
}

int32_t test_cmp2(const void *_a, const void *_b) {
		struct st *sa = (struct st *)_a;
		struct st *sb = (struct st *)_b;
		if (sa && sb) {
				find_num ++;
				return strcmp(sa->c, sb->c);
		}
		return 1;
}

void* test_copy(void* s, void* match)
{
		// GH_UNUSED(match)
		struct st *d;
		if (!s)
				return NULL;

		struct st *sp = (struct st *)s;

		d = (struct st *)malloc(sizeof(*d));
		memset(d, 0, sizeof(*d));

		if (!match) {
				d->a = sp->a;
				d->b = sp->b;
				d->c = (char *)malloc(strlen(sp->c) + 1);
				memset(d->c, 0, strlen(sp->c)+1);
				strncpy(d->c, sp->c, strlen(sp->c));
		} else {
				struct st *m = (struct st*) match;
				if (!strcmp(m->c, sp->c)) {
						d->a = sp->a;
						d->b = sp->b;
						d->c = (char *)malloc(strlen(sp->c) + 1);
						memset(d->c, 0, strlen(sp->c)+1);
						strncpy(d->c, sp->c, strlen(sp->c));
				} else {
						free (d);
						d = NULL;
				}
		}

		return (void*)d;
}


int main(int argc, char **argv)
{
		GH_UNUSED(argc)
				GH_UNUSED(argv)
				char c[][5] = {"fff", "bbb", "ggg", "eee", "ddd", "aaa", "ccc"};
		int i, idx;
		struct st *s;
#if 0
		gh_list_t gl;

		//gh_list_init(&gl, 16, free);
		gh_list_init(&gl, 16, test_free);
#else
		gh_list_t *gl;
		gl = gh_list_new(16, test_free);
		gl->gh_flags |= GH_LIST_F_UNIQUE;
#endif
		printf("======== test 添加与扩展   ==========\n");
		for (i = 0; i < 20; i ++) {
				s = (struct st *)malloc(sizeof(struct st));
				memset(s, 0, sizeof(*s));
				s->a = i;
				s->b = (double)(3.1415 * i);
				idx = i % (sizeof(c)/sizeof(c[0]));
				s->c = (char *)malloc(strlen(c[idx]) + 1);
				memset(s->c, 0, strlen(c[idx])+1);
				strncpy(s->c, c[idx], strlen(c[idx]));

				// gh_list_add(gl, (void*)s);
				gh_list_add_noduplicate(gl, (void*)s, test_cmp2);
		}
		for (i = 0; i < 20; i ++) {
				struct st *sp = (struct st *)gh_list_elem(gl, i);
				if (NULL == sp) continue;

				printf("%d %f %s \n", sp->a, sp->b, sp->c);
		}
		printf("\n======== test delete ===============\n");
		/*test delete 1 */
		struct st *sd = (struct st *)gh_list_elem(gl, 3); /*delete index = 3*/
		printf("delete:   %d %f %s \n", sd->a, sd->b, sd->c);
		gh_list_remove(gl, sd);
		gh_list_free_cb(gl, sd);

		/*test delete 2*/
		struct st sd2;
		struct st* sdp;
		sd2.a = 2;            /*delete s->a = 2*/
		sdp = (struct st *)gh_list_remove_cmp(gl, (void *)&sd2, test_cmp);
		printf("delete:   %d %f %s \n", sdp->a, sdp->b, sdp->c);
		gh_list_free_cb(gl, sdp);

		/*test delete 3*/
		struct st sd3;
		sd3.c = strdup("ggg");            /*delete all s->c = "ggg"*/
		int32_t cnt = gh_list_multi_remove_cmp(gl, (void *)&sd3, test_cmp2);
		printf("delete num:  %d  \n", cnt);

		printf("\n======== test 排序  ==========\n");
		gh_list_sort(gl, test_cmp2);

		for (i = 0; i < 20; i ++) {
				struct st *sp = (struct st *)gh_list_elem(gl, i);
				if (NULL == sp) continue;

				printf("%d %f %s \n", sp->a, sp->b, sp->c);
		}

		printf("\n======== test find 1 : 用排序查找 ==========\n");
		find_num = 0;
		free(sd3.c);
		sd3.c = strdup("fff");  
		struct st *sf = (struct st *)gh_list_find (gl, &sd3, test_cmp2);
		if (sf)
				printf("find %d %f %s \n", sf->a, sf->b, sf->c);
		printf("find num: %d\n", find_num);

		printf("\n======== test find 2: 不用排序查找 =========\n");
		find_num = 0;
		gl->gh_flags &= ~GH_LIST_F_SORTED;
		sf = (struct st *)gh_list_find (gl, &sd3, test_cmp2);
		if (sf)
				printf("find %d %f %s \n", sf->a, sf->b, sf->c);
		printf("find num: %d\n", find_num);

		printf("\n======== test copy 1: 默认拷贝函数 =========\n");
		gh_list_t *cp;
		cp = gh_list_copy(gl, NULL, NULL);
		for (i = 0; i < 20; i ++) {
				struct st *sp = (struct st *)gh_list_elem(cp, i);
				if (NULL == sp) continue;

				printf("%d %f %s \n", sp->a, sp->b, sp->c);
		}

		printf("\n==test copy 2: 根据自己定义数据结构编写拷贝函数 =====\n");
		gh_list_t *cp2;
		cp2 = gh_list_copy(gl, test_copy, NULL);
		for (i = 0; i < 20; i ++) {
				struct st *sp = (struct st *)gh_list_elem(cp2, i);
				if (NULL == sp) continue;

				printf("%d %f %s \n", sp->a, sp->b, sp->c);
		}
		gh_list_free(cp2);

		printf("\n==test copy 3: 复制相匹配的数据 =====\n");
		gh_list_t *cp3;
		free(sd3.c);
		sd3.c = strdup("aaa");  
		cp3 = gh_list_copy(gl, test_copy, &sd3);
		for (i = 0; i < 20; i ++) {
				struct st *sp = (struct st *)gh_list_elem(cp3, i);
				if (NULL == sp) continue;

				printf("%d %f %s \n", sp->a, sp->b, sp->c);
		}
		gh_list_free(cp3);

		gh_list_dump ("test", gl);

		gh_list_free(gl);
		free(sd3.c);
		return 0;
}


#endif



