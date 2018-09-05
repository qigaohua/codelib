#ifndef __JFIND_MONITOR_CACHE_H__
#define __JFIND_MONITOR_CACHE_H__
/**
 * monitor cache 
 * 用于大量数据的统计，根据data中相关点来计算hash值
 * 根据hash值定位到对应的monitor[hash]上
 * hash冲突时使用链式方法解决
 *
 * 统计元素data有 copy、dup、print、cmp方法
 * 
 */
#include <pthread.h>

#define MCACHE_ENTRY_SIZE (1024*1024)
#define MCACHE_ENTRY_MASK (MCACHE_ENTRY_SIZE - 1)



struct jcache {
	void* data;
	struct jcache* next;
};

struct jentry {
	pthread_mutex_t mutex;
	struct jcache *cache;
};

struct jmonitor_cache {
	int (*hashcode)(void*);
	int (*cache_cmp)(void*, void*);
	void* (*cache_dup)(void*);
	void (*cache_free)(void*);
	int (*cache_copy)(void *, void *);
	void (*cache_print)(void*);
	struct jentry monitor[MCACHE_ENTRY_SIZE];
};


struct jmonitor_cache* init_monitor_cache();

int add_monitor_cache(struct jmonitor_cache* jmt, void* data);

void free_monitor_cache(struct jmonitor_cache* jmt);

void show_monitor_cache(struct jmonitor_cache* jmt);

#endif //__JFIND_MONITOR_CACHE_H__
