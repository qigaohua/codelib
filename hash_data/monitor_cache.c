#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>
#include "hash.h"
#include "monitor_cache.h"

struct jmonitor_cache* init_monitor_cache()
{
	int i;
	struct jmonitor_cache* jmt = NULL;
	
	jmt = malloc(sizeof(struct jmonitor_cache));
	if(jmt) {
		memset(jmt, 0, sizeof(struct jmonitor_cache));

		for(i = 0; i < MCACHE_ENTRY_SIZE; i++) {
			jmt->monitor[i].cache = NULL;
			pthread_mutex_init(&(jmt->monitor[i].mutex), NULL);
		}
	}
	return jmt;
}

int add_monitor_cache(struct jmonitor_cache* jmt, void* data)
{
	int hash = 0;
	struct jentry* entry = NULL;
	struct jcache* tmp = NULL;

	if(jmt == NULL || data == NULL) {
		return -1;
	}
	
	hash = jmt->hashcode(data);

	//printf("hash %d\n", hash);
	entry = &jmt->monitor[hash];
	pthread_mutex_lock(&entry->mutex);

	if (entry) {
		tmp = entry->cache;
		while(tmp) {
			if(jmt->cache_cmp(tmp->data, data) == 0) {
				//相同
				jmt->cache_copy(tmp->data, data);
				pthread_mutex_unlock(&entry->mutex);
				return 0;
			}
			tmp = tmp->next;
		}
	}
	//printf("cache_cmp not found\n");
	tmp = jmt->cache_dup(data);
	if(tmp) {
		tmp->next = (struct jcache*)entry->cache;
		entry->cache = (void*)tmp;
	}
	
	pthread_mutex_unlock(&entry->mutex);
	return 0;
}

void free_monitor_cache(struct jmonitor_cache* jmt)
{
	int i;
	struct jcache* cache;
	struct jentry *entry;

	if(jmt) {
		for(i = 0; i < MCACHE_ENTRY_SIZE; i++) {
			entry = &jmt->monitor[i];
			pthread_mutex_lock(&entry->mutex);
			cache = entry->cache;
			while(cache) {
				struct jcache* next = cache->next;
				//jmt->cache_print(cache->data);
				jmt->cache_free(cache->data);
				free(cache);
				cache = next;
			}
			pthread_mutex_unlock(&entry->mutex);
			pthread_mutex_destroy(&entry->mutex);
		}
		free(jmt);
	}
}

void show_monitor_cache(struct jmonitor_cache* jmt)
{
	int i;
	struct jcache* cache;
	struct jentry *entry;

	//printf("%-3s %-15s %-5s %-40s\t%-40s\t %5s \t %-10s - %-10s\n", "hid", "ipv4", 
	//			"port", "sundomain", "topdomain", "count","firstfound", "lastfound");

	for(i = 0; i < MCACHE_ENTRY_SIZE; i++) {
		entry = &jmt->monitor[i];
		pthread_mutex_lock(&entry->mutex);
		cache = entry->cache;
		while(cache) {
			jmt->cache_print(cache->data);
			cache = cache->next;
		}
		pthread_mutex_unlock(&entry->mutex);
	}
}
