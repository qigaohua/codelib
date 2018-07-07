#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include "hash.h"
#include "jfind.h"
#include "monitor_cache.h"
#include "domain_cache.h"


int domain_hashcode(void* node)
{
	//ELF hash
    unsigned int hash = 0;
    unsigned int x;
    char *str;

    str = ((struct jdomaindata*)node)->domain;

    while(*str) {
        hash = (hash << 4) + *str++;
        x = hash & 0xF0000000L;
        if(x) {
            hash ^= x>>24;
            hash &= ~x;
        }
    }
    hash = hash & 0x7FFFFFFF;
    return hash &= MCACHE_ENTRY_MASK;
}

int domain_cmp(void *node, void *cache)
{
	struct jdomaindata *v1, *v2;
	if(node == NULL || cache == NULL) {
		return -1;
	}

	v1 = (struct jdomaindata*) node;
	v2 = (struct jdomaindata*) cache;
	
	
	if(v1->domain  == NULL && v2->domain == NULL) {
		return 0;
	} else {
		if(v1->domain  != NULL && v2->domain != NULL) {
			if(strcmp(v1->domain , v2->domain) == 0) {
				return 0;
			}
		}
		return -1;
	}
	return -1;
}

void* domain_dup(void* src)
{
	struct jcache *node;
	struct jdomaindata *data, *ndata;

	if(src == NULL) {
		return NULL;
	}
	data = (struct jdomaindata*) src;

	node = malloc(sizeof(struct jcache));
	if(node == NULL) {
		return NULL;
	}

	memset(node, 0, sizeof(struct jcache));
	node->data = malloc(sizeof(struct jdomaindata));
	if(node->data == NULL) {
		free(node);
		return NULL;
	}
	ndata = (struct jdomaindata*)node->data;

	strcpy(ndata->domain, data->domain);
	ndata->status = data->status;
	return (void*)node;
}

int domain_copy(void *node, void *cache)
{
	struct jdomaindata *v1, *v2;
	if(node == NULL || cache == NULL) {
		return -1;
	}

	v1 = (struct jdomaindata*) node;
	v2 = (struct jdomaindata*) cache;

	//copy
	v1->status = v2->status;

	return 0;
}


void domain_print(void* data)
{
	struct jdomaindata* mdata;

	if(data == NULL) {
		return;
	}

	mdata = (struct jdomaindata*)data;
	
	printf("domain: %-40s status: %d\n", mdata->domain, mdata->status);
	
	return;
}

void domain_free(void *data)
{
	if(data) {
		free(data);
		data = NULL;
	}
	return;
}


struct jmonitor_cache* init_domain_cache()
{
	struct jmonitor_cache* jmt;

	jmt = init_monitor_cache();

	if(jmt) {
		jmt->hashcode = domain_hashcode;
		jmt->cache_cmp = domain_cmp;
		jmt->cache_dup = domain_dup;
		jmt->cache_copy = domain_copy;
		jmt->cache_print = domain_print;
		jmt->cache_free = domain_free;
		return jmt;
	}

	return NULL;
}


int add_domain_cache(struct jmonitor_cache* jmt, void* data)
{
	return add_monitor_cache(jmt, data);
}

struct jdomaindata* find_domain_cache(struct jmonitor_cache *jmt, char *domain)
{
	struct jentry* entry = NULL;
	struct jcache* tmp = NULL;
	struct jdomaindata *mdata;
	unsigned int hash = 0;
    unsigned int x;
    char *str;

	if(jmt == NULL || domain == NULL) {
		return NULL;
	}
	str = domain;

    while(*str) {
        hash = (hash << 4) + (*str);
        x = hash & 0xF0000000L;
        if(x) {
            hash ^= x>>24;
            hash &= ~x;
        }
        str++;
    }
    hash = hash & 0x7FFFFFFF;
    hash &= MCACHE_ENTRY_MASK;

    entry = &jmt->monitor[hash];
    if (entry) {
		tmp = entry->cache;
		while(tmp) {
			if(tmp->data) {
				mdata = (struct jdomaindata*) tmp->data;
				if(strncasecmp(mdata->domain, domain, strlen(mdata->domain)) == 0) {
					return tmp->data;
				}
			}
			tmp = tmp->next;
		}
	}

	return NULL;
}

void free_domain_cache(struct jmonitor_cache* jmt)
{
	return free_monitor_cache(jmt);
}

void show_domain_cache(struct jmonitor_cache* jmt)
{
	return show_monitor_cache(jmt);
}
