/*
 *csim.c-使用C编写一个Cache模拟器，它可以处理来自Valgrind的跟踪和输出统计
 *息，如命中、未命中和逐出的次数。更换政策是LRU。
 * 设计和假设:
 *  1. 每个加载/存储最多可导致一个缓存未命中。（最大请求是8个字节。）
 *  2. 忽略指令负载（I），因为我们有兴趣评估trace.c内容中数据存储性能。
 *  3. 数据修改（M）被视为加载，然后存储到同一地址。因此，M操作可能导致两次缓存命中，或者一次未命中和一次命中，外加一次可能的逐出。
 * 使用函数printSummary() 打印输出，输出hits, misses and evictions 的数，这对结果评估很重要
*/
#include "cachelab.h"
//                    请在此处添加代码  
//****************************Begin*********************



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cachelab.h"

typedef struct {
    int valid;
    unsigned long tag;
    int lru_counter;
} CacheLine;

typedef struct {
    CacheLine *lines;
} CacheSet;

typedef struct {
    CacheSet *sets;
    int set_count;
    int lines_per_set;
} Cache;

int hit_count = 0, miss_count = 0, eviction_count = 0;
int lru_counter = 0;

void init_cache(Cache *cache, int s, int E) {
    cache->set_count = 1 << s;
    cache->lines_per_set = E;
    cache->sets = (CacheSet *)malloc(cache->set_count * sizeof(CacheSet));
    for (int i = 0; i < cache->set_count; i++) {
        cache->sets[i].lines = (CacheLine *)malloc(E * sizeof(CacheLine));
        for (int j = 0; j < E; j++) {
            cache->sets[i].lines[j].valid = 0;
            cache->sets[i].lines[j].tag = 0;
            cache->sets[i].lines[j].lru_counter = 0;
        }
    }
}

void free_cache(Cache *cache) {
    for (int i = 0; i < cache->set_count; i++) {
        free(cache->sets[i].lines);
    }
    free(cache->sets);
}

void access_cache(Cache *cache, unsigned long address, int s, int b) {
    unsigned long set_index = (address >> b) & ((1 << s) - 1);
    unsigned long tag = address >> (s + b);
    CacheSet *set = &cache->sets[set_index];
    int hit = 0, empty_line = -1, lru_line = 0;

    for (int i = 0; i < cache->lines_per_set; i++) {
        if (set->lines[i].valid && set->lines[i].tag == tag) {
            hit = 1;
            set->lines[i].lru_counter = ++lru_counter;
            break;
        }
        if (!set->lines[i].valid && empty_line == -1) {
            empty_line = i;
        }
        if (set->lines[i].lru_counter < set->lines[lru_line].lru_counter) {
            lru_line = i;
        }
    }

    if (hit) {
        hit_count++;
    } else {
        miss_count++;
        if (empty_line != -1) {
            set->lines[empty_line].valid = 1;
            set->lines[empty_line].tag = tag;
            set->lines[empty_line].lru_counter = ++lru_counter;
        } else {
            eviction_count++;
            set->lines[lru_line].tag = tag;
            set->lines[lru_line].lru_counter = ++lru_counter;
        }
    }
}

void simulate_cache(const char *trace_file, Cache *cache, int s, int b) {
    FILE *file = fopen(trace_file, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open trace file %s\n", trace_file);
        exit(1);
    }

    char operation;
    unsigned long address;
    int size;
    while (fscanf(file, " %c %lx,%d", &operation, &address, &size) > 0) {
        if (operation == 'I') continue;
        if (operation == 'L' || operation == 'S') {
            access_cache(cache, address, s, b);
        } else if (operation == 'M') {
            access_cache(cache, address, s, b);
            access_cache(cache, address, s, b);
        }
    }

    fclose(file);
}

int main(int argc, char **argv)
{   
    int s = 0, E = 0, b = 0;
    char *trace_file = NULL;
    int opt;

    while ((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (opt) {
            case 's': s = atoi(optarg); break;
            case 'E': E = atoi(optarg); break;
            case 'b': b = atoi(optarg); break;
            case 't': trace_file = optarg; break;
            default:
                fprintf(stderr, "Usage: %s -s <s> -E <E> -b <b> -t <tracefile>\n", argv[0]);
                exit(1);
        }
    }

    if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
        fprintf(stderr, "Usage: %s -s <s> -E <E> -b <b> -t <tracefile>\n", argv[0]);
        exit(1);
    }

    Cache cache;
    init_cache(&cache, s, E);
    simulate_cache(trace_file, &cache, s, b);
    free_cache(&cache);

    printSummary(hit_count, miss_count, eviction_count); //输出hit、miss和evictions数量统计 
    return 0;
}
//****************************End**********************#