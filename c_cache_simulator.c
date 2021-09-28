#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
typedef struct line {
    int valid_bit;
    int tag;
    int modified;
    struct line* next;
} line;
typedef struct set {
    struct set* next;
    line* first_line;
} set;
typedef struct cache {
    set* set_list;
    int cur_access_num;
} cache;
int hits = 0;
int misses = 0;
int evictions = 0;
cache* initialize_cache(int s, int E, int b) {
    cache* empty_cache = malloc(sizeof(cache));
    empty_cache->set_list = malloc(sizeof(set));
    set* temp_set = empty_cache->set_list;
    temp_set->next = NULL;
    temp_set->first_line = malloc(sizeof(line));
    line* temp_line = empty_cache->set_list->first_line;
    temp_line->valid_bit = 0;
    for(int d = 0; d < E - 1; d++) {
        temp_line->next = malloc(sizeof(line));
        temp_line = temp_line->next;
        temp_line->valid_bit = 0;
        temp_line->next = NULL;
    }
    for(int i = 0; (i < ((1 << s) - 1)); i++) {
        temp_set->next = malloc(sizeof(set));
        temp_set = temp_set->next;
        temp_set->first_line = malloc(sizeof(line));
        line* temp_line2 = temp_set->first_line;
        temp_line2->valid_bit = 0;
        for(int j = 0; j < E - 1; j++) {
            temp_line2->next = malloc(sizeof(line));
            temp_line2 = temp_line2->next;
            temp_line2->valid_bit = 0;
            temp_line2->next = NULL;
        }
        temp_set->next = NULL;
    }
    empty_cache->cur_access_num = 0;
    return empty_cache;
}
int get_set(int mem_address, int s, int b) {
    int mask = 1 << 31;
    mask = mask >> (31 - s);
    return (mem_address >> b) & (~mask); 
}
int get_tag(int mem_address, int s, int b) {
    int mask = 1 << 31;
    mask = mask >> ((b + s) - 1);
    int to_return = ~mask & (mem_address >> (b + s));
    return to_return;
}
void free_set(set* set_to_free) {
    line* cur_line = set_to_free->first_line;
    line* temp_line = set_to_free->first_line;
    while(cur_line) {
        temp_line = temp_line->next;
        free(cur_line);
        cur_line = temp_line;
    }
    free(set_to_free);
}
void free_cache(cache* cache) {
    set* cur_set = cache->set_list;
    set* temp_set = cache->set_list;
    while(cur_set) {
        temp_set = temp_set->next;
        free_set(cur_set);
        cur_set = temp_set;
    }
    free(cache);
}
void insert_line(cache* cache, set* set, int E, int line_tag) {
    line* cur_line = set->first_line;
    if (cur_line->valid_bit == 0) {
        cur_line->tag = line_tag;
        cur_line->valid_bit = 1;
        cache->cur_access_num++;
        cur_line->modified = cache->cur_access_num;
        misses++;
        return;
    }
    for(int i = 0; i < E - 1; i++) {
        cur_line = cur_line->next;
        if (cur_line->valid_bit == 0) {
            cur_line->tag = line_tag;
            cur_line->valid_bit = 1;
            cache->cur_access_num++;
            cur_line->modified = cache->cur_access_num;
            misses++;
            return;
        }
    }
    cur_line = set->first_line;
    line* lru_line = cur_line;
    for(int i = 0; i < E - 1; i++) {
        cur_line = cur_line->next;
        if (cur_line->modified < lru_line->modified) {
            lru_line = cur_line;
        }
    }
    lru_line->tag = line_tag;
    cache->cur_access_num++;
    lru_line->modified = cache->cur_access_num;
    misses++;
    evictions++;
    return;
}
void simulate_cache(cache* cache, char access_type, int mem_address, int s, int E, int b) {
    
    set* cur_set = cache->set_list;
    int set_index = get_set(mem_address, s, b);
    for(int i = 0; i < set_index; i++) {
        cur_set = cur_set->next;
    }
    line* cur_line = cur_set->first_line;
    int line_tag = get_tag(mem_address, s, b);
    if(access_type == 'L') {
        if(cur_line->valid_bit == 1 && cur_line->tag == line_tag) {
            hits++;
            cache->cur_access_num++;
            cur_line->modified = cache->cur_access_num;
            return;
        }
        for(int j = 0; j < E - 1; j++) {
            cur_line = cur_line->next;
            if(cur_line->valid_bit == 1 && cur_line->tag == line_tag) {
                hits++;
                cache->cur_access_num++;
                cur_line->modified = cache->cur_access_num;
                return;
            }
        }
        insert_line(cache, cur_set, E, line_tag);
    }
    else if (access_type == 'S') {
        if(cur_line->valid_bit == 1 && cur_line->tag == line_tag) {
            hits++;
            cache->cur_access_num++;
            cur_line->modified = cache->cur_access_num;
            return;
        }
        for(int j = 0; j < E - 1; j++) {
            cur_line = cur_line->next;
            if(cur_line->valid_bit == 1 && cur_line->tag == line_tag) {
                hits++;
                cache->cur_access_num++;
                cur_line->modified = cache->cur_access_num;
                return;
            }
        }
        insert_line(cache, cur_set, E, line_tag);
    }
    else if (access_type == 'M') {
        simulate_cache(cache, 'L', mem_address, s, E, b);
        simulate_cache(cache, 'S', mem_address, s, E, b);
    }
    else {
        return;
    }
}
         
int main(int argc, char* argv[])
{
    char curr_str[16];
    char access_type;
    int mem_address;
    int opt;
    int s;
    int E;
    int b;
    char* file;
    while((opt = getopt(argc, argv,"s:E:b:t:" )) != -1) {
        switch(opt) {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                file = optarg;
                break;
        }
    }
    cache* my_cache = initialize_cache(s, E, b);
    FILE* fp;
    fp = fopen(file, "r");
    while (fgets(curr_str, 16, fp)) {
        sscanf(curr_str, " %c %x", &access_type, &mem_address);
        simulate_cache(my_cache, access_type, mem_address, s, E, b);
    }
    fclose(fp);
    free_cache(my_cache);
    printSummary(hits, misses, evictions);
    return 0;
}
