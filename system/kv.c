#include <xinu.h>
#include <segmem.h>
#include <kv.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

dataItem* cache[CACHE_SIZE];
dataItem* garbage[4096];
int garbage_pointer = 0;
cache_info info = {0, 0, 0, CACHE_SIZE, 0, 0};
long lru_count = 0;
int max_key_size = 64;
int max_value_size = 1024;

int hash(char* key){
	int hash = 1;
	int i =0;
	for (i = 0; i < strlen(key); i++) {
		int temp = key[i];
		hash = (hash*3) % 1000 + temp;
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
	//printf("Cannot get value for %s\n",key);
	return NULL;
}

int kv_set(char* key, char* value){
        int hash_val = hash(key);
	int temp = hash_val;
	int c=0;
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
			if(strcmp(cache[temp]->key, "odzuzdge")==0)
			{
				printf("LRU key odzuzdge count = %d \n",cache[temp]->count);
				return 1;
			}
			return 0;

		}
		temp ++;
		temp%= CACHE_SIZE;
		c++;
	}while(temp != hash_val);
	temp = hash_val;
	// If key not found so look for first empty slot to set the k-v pair
	if(info.num_keys <  CACHE_SIZE)
	{	
		do{
			if (cache[temp] == NULL){
				dataItem* x;
				

				x = (dataItem *) xmalloc(sizeof(long) + strlen(key) + strlen(value));
				if (x ==(dataItem *) SYSERR ){
					return 1;
				}
				x->count = ++lru_count;
				x->key = key;
				x->data = value;
				//printf("key to set :%s\n", );
				cache[temp] = x;
				info.num_keys++;
				info.total_set_success++;
				return 0;
			}
			temp++;
			temp %= CACHE_SIZE ;
		}while(temp != hash_val);

	}
	// If cache is full look for LRU item.
	// Acquire new memory block.
	else{
		
		long min_lru = LONG_MAX;
		int min_hash = temp;
		do{
			if (cache[temp]->count < min_lru){
				min_lru = cache[temp]->count;
				info.total_evictions++;
				min_hash = temp;
			} 
			temp++;
			temp %= CACHE_SIZE ;
		}while(temp != hash_val);
		dataItem* x;
		x = (dataItem *) xmalloc(sizeof(long) + strlen(key) + strlen(value));
		if (x ==(dataItem *) SYSERR ){
		return 1;
		}
		x->count = ++lru_count;
		x->key = key;
		x->data = value;
		//printf("Removing key: %s",cache[min_hash]->cal);
		info.total_set_success++;
		garbage[garbage_pointer++] = cache[min_hash];
		cache[min_hash] = x;
		// xmalloc cache[min_hash]
		return 0;

	}

	// If cache
	return 1;
}

bool kv_delete(char* key){
 	int hash_val = hash(key);
        int temp = hash_val;
        do{

                if(cache[temp] != NULL && (strcmp(cache[temp]->key, key)==0)){
			info.num_keys--;
			xfree(cache[temp]);
			cache[temp]  = NULL;
                        return TRUE;
                }
                temp ++;
                temp%= CACHE_SIZE;
        }while(temp != hash_val);
	return FALSE;
}

void kv_reset(){
	int i = 0;
	int c = 0;
	int d =0;
	for (i=0; i < CACHE_SIZE; i++){
		if (cache[i] != NULL)
		{
			//printf("Deleting cache mem\n");
			xfree(cache[i]);
			cache[i] =NULL;
			c++;
		}

	}
	for (i = 0; i < 4096; i++){
		if (garbage[i] != NULL){
			//printf("Deleting garbage  mem\n");
			xfree(garbage[i]);
			garbage[i] = NULL;
			d++;
		}
	}
	info.total_hits = 0;
	info.total_accesses =0;	
	info.total_set_success = 0;
	info.num_keys = 0;	
	info.total_evictions =0;
	garbage_pointer =0;
	lru_count = 0;		
}
int get_cache_info(char* kind){
	if(strcmp(kind, "total_hits")==0)
	{
		return info.total_hits;
	}if(strcmp(kind, "total_accesses")==0)
	{
		return info.total_accesses;
	}if(strcmp(kind, "total_set_success")==0)
	{
		return info.total_set_success;
	}if(strcmp(kind, "num_keys")==0)
	{
		return info.num_keys;
	}if(strcmp(kind, "total_evictions")==0)
	{
		return info.total_evictions;
	}

}
int kv_init(){
	xmalloc_init();
	return 0;
}
char** most_popular_keys(int k){
	int i = 0, j = 0, keys_index = 0;
	int prev_max = INT_MAX;
	char** keys  = xmalloc(sizeof( *keys) * k); 

	for(i=0; i< k; i++){
		int max = 0;
		char* keyasd[64];
		for(j=0; j< CACHE_SIZE; j++ )
		{
			if(cache[j] !=NULL){
				if (cache[j]->count > max && cache[j]->count < prev_max){
					max = cache[j]->count;
					//printf("%s\n",cache[j]->key);
					int c=0;
					while (cache[j]->key[c] != '\0') {
 						keyasd[c] =cache[j]->key[c];
 						c++;
  					}
				}
			}
		}
		keys[keys_index]= xmalloc(sizeof *keys[keys_index] * 64);
		int c=0;
		while (keyasd[c] != '\0') {
 			keys[keys_index][c] =keyasd[c];
 			c++;
  		}
		prev_max = max;
		keys_index++;
	}
	return keys;
}
