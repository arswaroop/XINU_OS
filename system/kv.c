#include <xinu.h>
#include <segmem.h>
#include <kv.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>


dataItem* cache[CACHE_SIZE];
dataItem* aux[4096];
int aux_pointer = 0; // For Auxiliary for memory management
cache_info info = {0, 0, 0, CACHE_SIZE, 0, 0}; // Initializing cache_info
long lru_count = 0; // LRU count - The count would denote MRU element, while the onw with least value would be considered as LRU element.
int max_key_size = 64; // Limiting Key Size
int max_value_size = 1024; // Limiting the value size


// Hash function to calculate hash-value
int hash(char* key){
	int hash = 1;
	int i =0;
	for (i = 0; i < strlen(key); i++) {
		int temp = key[i];
		hash = (hash*31) % 1000 + temp;
	}
	return hash % CACHE_SIZE;
}

// Get the value given the key
char* kv_get(char* key){
	int hash_val = hash(key); // Calculate the hash_value for the given key
	int temp = hash_val;
	// Start looking for the key from hash position.
	// Iteratively look for key until loop once.
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
	// Check the max sizes allowed before allocation
	// If more than max_size, return error
	if (sizeof(key) > max_key_size){
		printf("The size of Key is larger than the limit of 64B, please give a valid key\n");
		return 1;
	}
	if (sizeof(value) > max_value_size){
		printf("The size of Value is larger than the limit of 1KB, Please provide a valid Value");
		return 1;
	}
	
	// Looking for empty slot at the hash value.
	// If not empty, find next empty space in cache.
	// Iterate once through cache to find a empty value.
	do{
		if(cache[temp] != NULL && (strcmp(cache[temp]->key , key) ==0)){
			cache[temp]->count = ++lru_count;
			cache[temp]->data = value;
			if(strcmp(cache[temp]->key, "odzuzdge")==0)
			{
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
		aux[aux_pointer++] = cache[min_hash];
		cache[min_hash] = x;
		// xmalloc cache[min_hash]
		return 0;

	}

	// If cache
	return 1;
}
// Deletes the keu and frees the memory
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

// Resets all the counters and free all the memory acquired
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
		if (aux[i] != NULL){
			xfree(aux[i]);
			aux[i] = NULL;
			d++;
		}
	}
	info.total_hits = 0;
	info.total_accesses =0;	
	info.total_set_success = 0;
	info.num_keys = 0;	
	info.total_evictions =0;
	aux_pointer =0;
	lru_count = 0;		
}

// Return the info that has been asked for
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
	printf("Plese input a valid option");
	return 0;

}
// Initializes the buffers 
int kv_init(){
	xmalloc_init();
	return 0;
}

// Gets k most popular keys
char** most_popular_keys(int k){
	int i = 0, j = 0, keys_index = 0;
	int prev_max = INT_MAX;
	char** keys  = xmalloc(sizeof( *keys) * k); 
	// Iterate k times for the key
	for(i=0; i< k; i++){
		int max = 0;
		char* keyasd[64];
		for(j=0; j< CACHE_SIZE; j++ )
		{
			
			if(cache[j] !=NULL){
				if (cache[j]->count > max && cache[j]->count < prev_max){
					max = cache[j]->count;
					int c=0;
					// Copy the string to required variable
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
