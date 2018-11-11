#ifndef _KV_H_
#define _KV_H_

#define CACHE_SIZE 100
#define bool int
#define true 1
#define false 0
typedef struct dataItem{
	char * data;
	char* key;
	long count;
}dataItem;
typedef struct cache_info{
	int total_hits;
	int total_accesses;
	int total_set_success;
	int cache_size;
	int num_keys;
	int total_evictions;	
}cache_info;


// Initialize Cache
extern dataItem* cache[CACHE_SIZE];
// KV function
char* kv_get(char* key);
int kv_delete(char* key);
int kv_set(char* key, char* value);
void kv_reset();
int kv_init();

#endif
