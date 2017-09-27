/*
 *一种编程方法：存储
 *
 * 伪代码
 * */

typedef struct {
    long counter;
} atomic_64_t;

#define ATOMIC_64_INIT(i)  { (i) }

static inline long atomic_64_read(const atomic_64_t *v)
{
    return (*(volatile long *)&(v)->counter);
}

static inline void atomic_64_set(atomic_64_t *v, long i)
{
    v->counter = i;
}

struct jfind_conf {
	int illegalweb_list_polity; // 违法网站列表命中自动处置
	int illegalweb_icp_polity; // 未备案域名自动处置
	int illegalweb_icp_ip_polity; // 未备案IP域名自动处置
	int basic_monitor_polity; // basic monitor data default off
	int jfind_interval; // set 60 sec as default
	atomic_64_t domain_postfix;
	atomic_64_t illegalweb_domain_list; //违法违规网站列表
	atomic_64_t illegalweb_ip_list; //违法违规IP列表
	atomic_64_t domain_icp_list; //域名备案信息列表
	atomic_64_t user_domain_list; //用户登记域名列表 {domain, userid}
	atomic_64_t user_iptrans_list; //用户登记IPv4列表 {userid, {domain}}
	atomic_64_t house_monitor_data; // idc_housemonitor data cache
	atomic_64_t illegalweb_monitor_data; // idc_sitemonitor data cache
	atomic_64_t resource_monitor_data; // find_domain_ip_list data cache
};
struct jfind_conf global_conf;

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

struct jhousedata {
	int id;
	int hid;
	uint32_t ipv4;
	uint16_t port;
	char *domain;
	u8 protocol;
	u8 block;
	u8 currentState;
	uint16_t illegalType;
	uint16_t regError;
	uint16_t icpError;
};

struct jmonitor_cache* init_monitor_cache();

int add_monitor_cache(struct jmonitor_cache* jmt, void* data);

void free_monitor_cache(struct jmonitor_cache* jmt);

void show_monitor_cache(struct jmonitor_cache* jmt);

struct jmonitor_cache* init_housemonitor_cache()
{
	struct jmonitor_cache* jmt;

	jmt = init_monitor_cache();

	if(jmt) {
		jmt->hashcode = house_hashcode;
		jmt->cache_cmp = house_cmp;
		jmt->cache_dup = house_dup;
		jmt->cache_copy = house_copy;
		jmt->cache_print = house_print;
		jmt->cache_free = house_free;
		return jmt;
	}

	return NULL;
}

int house_hashcode(void* node)
{
	int hash = 0, i;
	int domainhash = 0;

	struct jhousedata* data;

	if(node == NULL) {
		return -1;
	}


	data = (struct jhousedata*) node;
	if(data->domain) {
		for(i = 0; data->domain[i] != '\0'; i++) {
			domainhash = (domainhash << 7) + (domainhash << 1) + domainhash + data->domain[i];
		}
	}
	hash = jhash_3words(data->ipv4, domainhash, data->protocol, JHASH_INITVAL);
	hash &= MCACHE_ENTRY_MASK;
	return hash;
}

int house_cmp(void *node, void *cache)
{
	struct jhousedata *v1, *v2;
	if(node == NULL || cache == NULL) {
		return -1;
	}

	v1 = (struct jhousedata*) node;
	v2 = (struct jhousedata*) cache;
	
	if(v1->ipv4 == v2->ipv4 && 
			v1->protocol == v2->protocol) {
		if(v1->domain == NULL && v2->domain == NULL) {
			return 0;
		} else {
			if(v2->domain != NULL && v1->domain != NULL) {
				if(strcmp(v2->domain, v1->domain) == 0) {
					return 0;
				}
			}
			return -1;
		}
	}
	return -1;
}

void* house_dup(void* src)
{
	struct jcache *node;
	struct jhousedata *data, *ndata;

	if(src == NULL) {
		return NULL;
	}
	data = (struct jhousedata*) src;

	node = malloc(sizeof(struct jcache));
	if(node == NULL) {
		return NULL;
	}

	memset(node, 0, sizeof(struct jcache));
	node->data = malloc(sizeof(struct jhousedata));
	if(node->data == NULL) {
		free(node);
		return NULL;
	}
	ndata = (struct jhousedata*)node->data;

	ndata->id = data->id;
	ndata->hid = data->hid;
	ndata->ipv4 = data->ipv4;
	ndata->port = data->port;
	ndata->domain = data->domain == NULL ? NULL : strdup(data->domain);
	ndata->protocol = data->protocol;
	ndata->block = data->block;
	ndata->currentState = data->currentState;
	ndata->illegalType = data->illegalType;
	ndata->regError = data->regError;
	return (void*)node;
}

int house_copy(void *node, void *cache)
{
	return 0;
}

void house_print(void* data)
{
	struct jhousedata* mdata;

	char addr[32];
	if(data == NULL) {
		return;
	}
	mdata = (struct jhousedata*)data;
	//if(mdata->count > 100) {
		printf("%3d %3d %s %5d %-40s\t%2d %2d %2d %3d %d\n", 
			mdata->id, mdata->hid, ip2string(mdata->ipv4, addr), mdata->port, 
			mdata->domain, mdata->protocol, mdata->block, mdata->currentState,
			mdata->illegalType, mdata->regError);
	//}
	return;
}



void house_free(void *data)
{
	struct jhousedata *mdata;
	mdata = (struct jhousedata*) data;

	if(mdata) {
		if(mdata->domain) {
			free(mdata->domain);
		}
		free(mdata);
	}
}

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

int main()
{
	struct jmonitor_cache *house_monitor_data = NULL, *house_monitor_data_old = NULL;
	struct jhousedata data;

	house_monitor_data = init_housemonitor_cache();
	if(house_monitor_data == NULL) {
		return -1;
	}

	ret = add_monitor_cache(house_monitor_data, (void*)&data2);
	if(ret) {
		printf("[%s:%d] add house monitor data error.\n", __FILE__, __LINE__);
	}

	house_monitor_data_old = (struct jmonitor_cache*)atomic_64_read(&(global_conf.house_monitor_data));
	atomic_64_set(&global_conf.house_monitor_data, (u64)house_monitor_data);
	house_monitor_data = house_monitor_data_old;
error:
#ifdef __DEBUG__
	printf("[%s:%d] load house monitor data [count %d]\n", __FILE__, __LINE__, count);
#endif
	free_monitor_cache(house_monitor_data);
	return 0;
}
