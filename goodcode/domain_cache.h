#ifndef __JFIND_DOMAIN_CACHE_H__
#define __JFIND_DOMAIN_CACHE_H__
#include "monitor_cache.h"

struct jdomaindata {
	char 	domain[255];
	int 	status;
};

struct jmonitor_cache* init_domain_cache();

int add_domain_cache(struct jmonitor_cache *jmt, void* data);

struct jdomaindata* find_domain_cache(struct jmonitor_cache *jmt, char *domain);

void free_domain_cache(struct jmonitor_cache *jmt);

void show_domain_cache(struct jmonitor_cache *jmt);



#endif //__JFIND_DOMAIN_CACHE_H__
