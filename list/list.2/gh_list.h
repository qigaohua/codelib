#ifndef _GHLIST_H
#define _GHLIST_H
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


typedef  signed char       int8_t;   
typedef  unsigned char  uint8_t;  
typedef  short          int16_t; 
typedef  unsigned short uint16_t; 
typedef  int            int32_t; 
typedef  unsigned int   uint32_t;

typedef void (*func)(void*);


#ifndef likely
#define likely(x)   __builtin_expect((x),1)
#endif
#ifndef unlikely
#define unlikely(x) __builtin_expect((x),0)
#endif

#define GH_UNUSED(val) (void)val; 


#define logerr(fmt, ...) \
            printf("[%s][%d] "#fmt"\r\n", \
	                       __FILE__, __LINE__, ##__VA_ARGS__)

#define check_param(p, type) \
		do { \
		    if (!(p)) { \
			    logerr("Invaild parameter"); \
			    return (type); \
			} \
		} while(0)

#define GH_LIST_FOREACH(gl, index, elem) \
		for (index = 0; (elem = gh_list_elem(gl, index)); index ++)

typedef struct gh_list_s {
	int32_t gh_size;
	int32_t gh_cnt;
	void **elems;
	func cb_free;
    uint32_t gh_flags;	
#define GH_LIST_F_ALLOCATED 0x0001  /* The rd_list_t is allocated, will be free on destroy*/
#define GH_LIST_F_SORTED 0x0010     /* Sort*/
#define GH_LIST_F_UNIQUE 0x0100     /* Don't allow duplicates*/
#define GH_LIST_F_NOTGROWS 0x1000   /* Don't allow grow*/
} gh_list_t, *gh_list_p;


#define gh_free free
static __inline void* gh_realloc(void *ptr, size_t size)
{
    void* p = realloc(ptr, size);
	if (!p) {
		perror("realloc error");
		exit(0);
	}
	return p;
}

static __inline void* gh_malloc(size_t size)
{
    void* p = malloc(size);
	if (!p) {
		perror("malloc error");
		exit(0);
	}
	return p;
}



gh_list_t* gh_list_init(gh_list_t *gl, int32_t size, func cb_free);
int gh_list_grow (gh_list_t* gl, int32_t size);
gh_list_t* gh_list_new (int32_t size, func cb_free);
int32_t gh_list_sort (gh_list_t *gl, 
				      int32_t (*cmp)(const void *_a, const void *_b));
gh_list_t* gh_list_new (int32_t size, func cb_free);
void*  gh_list_elem (gh_list_t *gl, int32_t i);
void* gh_list_find (gh_list_t *gl, const void *elem,
				    int32_t (*cmp)(const void *_a, const void *_b));
void gh_list_free_cb (gh_list_t *gl, void *ptr);
int32_t gh_list_add_noduplicate (gh_list_t *gl, void *elem, 
				 int32_t (*cmp)(const void *_a, const void *_b));
int32_t gh_list_add (gh_list_t *gl, void *elem);
void* gh_list_remove_index(gh_list_t *gl, int32_t idx);
void* gh_list_remove (gh_list_t *gl, void *elem);
void* gh_list_remove_cmp (gh_list_t *gl, void *elem, 
				         int32_t (*cmp)(void *_a, void *_b));
int32_t gh_list_multi_remove_cmp (gh_list_t *gl, void *elem, 
				         int32_t (*cmp)(const void *_a, const void *_b));
void __inline gh_list_clear (gh_list_t *gl);
int32_t __inline gh_list_cnt (gh_list_t *gl);
int32_t __inline gh_list_empty (gh_list_t *gl);
void* gh_list_onlycopy_ptr (void *s, void *mt);
int32_t gh_list_copy_to (gh_list_t *dst, gh_list_t *src,
				      void* (*copy)(void* _d, void *_s),
					  void* match);
void gh_list_free (gh_list_t *gl);
gh_list_t* gh_list_copy (gh_list_t *src,
				      void* (*copy)(void* _d, void *_s),
					  void* match);

#endif
