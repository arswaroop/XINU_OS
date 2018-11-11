#include <xinu.h>
#include <segmem.h>
#include <kv.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define LONG_MAXIMUM 9223372036854775807
dataItem* cache[CACHE_SIZE];
cache_info info = {0, 0, 0, CACHE_SIZE, 0, 0};
long lru_count = 0;
int max_key_size = 64;
int max_value_size = 1024;

int hash(char* key){
	int hash = 7;
	int i =0;
	for (i = 0; i < strlen(key); i++) {
		int temp = key[i];
		hash = hash*31 + temp;
	}
	return hash % CACHE_SIZE;
}
char* kv_get(char* key){
	
	int hash_val = hash(key);
	int temp = hash_val;
	do{	if(cache[temp] != NULL){
			if(strcmp(cache[temp]->key, key) == 0){
				cache[temp]->count = ++lru_count;
				info.total_hits++;
				info.total_accesses++;
				return cache[hash_val]->data;
			}
		}
		temp++;
		temp %= CACHE_SIZE;
	}while ( temp != hash_val);

	info.total_accesses++;
}

int kv_set(char* key, char* value){
        int hash_val = hash(key);
	int temp = hash_val;
	if (sizeof(key) > max_key_size){
		printf("The size of Key is larger than the limit of 64B, please give a valid key\n");
		return 1;
	}
	if (sizeof(value) > max_value_size){
		printf("The size of Value is larger than the limit of 1KB, Please provide a valid Value");
		return 1;
	}
	// Look for value in the cache	
	do{	
		if(cache[temp] != NULL && (strcmp(cache[temp]->key , key) ==0)){
			cache[temp]->count = ++lru_count;
			cache[temp]->data = value;
			return 0;
		}
		temp ++;
		temp%= CACHE_SIZE;
	}while(temp != hash_val);


	// If key not found so look for first empty slot to set the k-v pair
	if(info.num_keys <  CACHE_SIZE)
	{
		do{
			if (cache[temp] == NULL){
				dataItem* x;
				x = (dataItem *) xmalloc(sizeof(long) + sizeof(key) + sizeof(value));
				x->count = ++lru_count;
				x->key = key;
				x->data = value;
				cache[temp] = x;
				info.num_keys++;
				return 0;
			}
			temp++;
			temp %= CACHE_SIZE ;
		}while(temp != hash_val);

	}
	// If cache is full look for LRU item.
	// Acquire new memory block.
	else{
		long min_lru = LONG_MAXIMUM;
		int min_hash = temp;
		do{
			if (cache[temp]->count < min_lru){
				min_lru = cache[temp]->count;
				info.total_evictions++;
				min_hash = temp;
			} 
		}while(temp != hash_val);
		dataItem* x;
		x = (dataItem *) xmalloc(sizeof(long) + sizeof(key) + sizeof(value));
		x->count = ++lru_count;
		x->key = key;
		x->data = value;
		cache[min_hash] = x;
		// xmalloc cache[min_hash]
		return 0;

	}

	// If cache
	return 1;
}

int kv_delete(char* key){
 	int hash_val = hash(key);
        int temp = hash_val;
        do{

                if(cache[temp] != NULL && (strcmp(cache[temp]->key, key)==0)){
			info.num_keys--;
			xfree(cache[temp]);
			cache[temp]  = NULL;
                        return 1;
                }
                temp ++;
                temp%= CACHE_SIZE;
        }while(temp != hash_val);
	return 0;
}

void kv_reset(){
	int i = 0;
	for (i=0; i < CACHE_SIZE; i++){
		xfree(cache[i]);
		cache[i] =NULL;
	}
}

int kv_init(){
	xmalloc_init();
	return 0;
}

