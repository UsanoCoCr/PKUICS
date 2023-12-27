/*
康子熙 2200017416
*/

#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

struct cache_line{
    int valid;
    int tag;
    int last_used_time;
};

typedef struct cache_line *cache_set;

cache_set *cache;

int s, E, b;
char *tracefile;
int hit_count = 0, miss_count = 0, eviction_count = 0;
char op;
long addr;
int size;
FILE *file;

int main(int argc, char *argv[]){
    int opt;
    while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1){
        switch(opt){
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
                file = fopen(optarg, "r");
                break;
            default:
                printf("wrong argument\n");
                exit(0);
        }
    }

    cache = (cache_set *)malloc(sizeof(cache_set) * (1 << s));
    for(int i = 0; i < (1 << s); i++){
        cache[i] = (cache_set)malloc(sizeof(struct cache_line) * E);
        for(int j = 0; j < E; j++){
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].last_used_time = -1;
        }
    }

    while(fscanf(file, "%s %lx,%d\n", &op, &addr, &size) != EOF){
        if(op == 'I'){
            continue;
        }
        int set_index = (addr >> b) & ((1 << s) - 1);
        int tag = addr >> (s + b);
        int hit = 0, miss = 0, eviction = 0;
        for(int i = 0; i < E; i++){
            if(cache[set_index][i].valid == 1 && cache[set_index][i].tag == tag){
                hit = 1;
                cache[set_index][i].last_used_time = 0;
                break;
            }
        }
        if(hit == 0){
            miss = 1;
            for(int i = 0; i < E; i++){
                if(cache[set_index][i].valid == 0){
                    cache[set_index][i].valid = 1;
                    cache[set_index][i].tag = tag;
                    cache[set_index][i].last_used_time = 0;
                    miss = 0;
                    break;
                }
            }
            if(miss == 1){
                eviction = 1;
                int max_time = 0, max_index = 0;
                for(int i = 0; i < E; i++){
                    if(cache[set_index][i].last_used_time > max_time){
                        max_time = cache[set_index][i].last_used_time;
                        max_index = i;
                    }
                }
                cache[set_index][max_index].tag = tag;
                cache[set_index][max_index].last_used_time = 0;
            }
        }
        for(int i = 0; i < E; i++){
            if(cache[set_index][i].valid == 1){
                cache[set_index][i].last_used_time++;
            }
        }
        if(hit == 1){
            hit_count++;
        }
        else{
            miss_count++;
        }
        if(eviction == 1){
            eviction_count++;
        }

        if(op == 'M'){
            hit_count++;
        }
    }
    printSummary(hit_count, miss_count, eviction_count);
    free(cache);
    return 0;
}