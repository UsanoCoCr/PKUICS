/*
 *乐睿承 2200017806
 */
#include "cachelab.h"
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

typedef struct cache_line
{
    int valid;     //有效位
    int tag;       //标记位
    int time_stamp; //时间戳
} Cache_line;//实现Cacheline 其中时间戳为了LRU策略访问最近最少而添加

typedef struct cache
{
    int S;
    int E;
    int B;
    Cache_line **line;
} Cache;//实现整个Cache，类似于一个二维数组

int hit_count = 0, miss_count = 0, eviction_count = 0; // 记录冲突不命中、缓存不命中
int verbose = 0;                                       //是否打印跟踪详细信息
char t[500000];
Cache *cache = NULL;

void Init_Cache(int s, int E, int b)
{
    int S = 1 << s;//2^s个组
    int B = 1 << b;//2^b个字节块大小
    cache = (Cache *)malloc(sizeof(Cache));//给Cache分配内存
    cache->S = S;
    cache->E = E;
    cache->B = B;
    cache->line = (Cache_line **)malloc(sizeof(Cache_line *) * S);
    for (int i = 0; i < S; i++)
    {
        cache->line[i] = (Cache_line *)malloc(sizeof(Cache_line) * E);
        for (int j = 0; j < E; j++)
        {
            cache->line[i][j].valid = 0; //初始时，高速缓存是空的，有效位为0
            cache->line[i][j].tag = -1;//标记位置-1
            cache->line[i][j].time_stamp = -1;//时间戳为-1
        }
    }//实现Cache的二维数组，S个组，每组E行
}

void free_Cache()
{
    int S = cache->S;
    for (int i = 0; i < S; i++)
    {
        free(cache->line[i]);
    }
    free(cache->line);
    free(cache);
}//释放Cache占用空间
int get_index(int op_s, int op_tag)
{
    for (int i = 0; i < cache->E; i++)
    {
        if (cache->line[op_s][i].valid > 0 && cache->line[op_s][i].tag == op_tag)
            return i;
    }
    return -1;
}//在第S组中寻找标记位相同的行，如果找到返回行下标
int find_LRU(int op_s)
{
    int max_index = -1;
    int max_stamp = INT_MIN;
    for(int i = 0; i < cache->E; i++){
        if(cache->line[op_s][i].time_stamp > max_stamp){
            max_stamp = cache->line[op_s][i].time_stamp;
            max_index = i;
        }
    }
    return max_index;
}//寻找最远访问的行
int is_full(int op_s)
{
    for (int i = 0; i < cache->E; i++)
    {
        if (cache->line[op_s][i].valid == 0)
            return i;
    }
    return -1;
}//判断第S组是否全部填满，如国未填满返回行下标
void update(int i, int op_s, int op_tag){
    cache->line[op_s][i].valid=1;
    cache->line[op_s][i].tag = op_tag;
    cache->line[op_s][i].time_stamp = 0;
}//更新第S组第i行缓存
void update_info(int op_tag, int op_s)
{
    int index = get_index(op_s, op_tag);
    if (index == -1)
    {
        miss_count++;
        if (verbose)
            printf("miss ");//未命中
        int i = is_full(op_s);//冷不命中
        if(i==-1){
            eviction_count++;
            if(verbose) printf("eviction");
            i = find_LRU(op_s);
        }
        update(i,op_s,op_tag);//如果冷不命中，替换空行；反之，则LRU策略替换
    }
    else{
        hit_count++;
        if(verbose)
            printf("hit");
        update(index,op_s,op_tag);    
    }//命中的情形
}//判断是否命中，以及后续操作，更新参数
void update_time_stamp()
{
	for(int i = 0; i < cache->S; ++i)
		for(int j = 0; j < cache->E; ++j)
            if(cache->line[i][j].valid==1)
                cache->line[i][j].time_stamp++;
}
void get_trace(int s, int E, int b)
{
    FILE *pFile;
    pFile = fopen(t, "r");
    if (pFile == NULL)
    {
        exit(-1);
    }
    char identifier;
    unsigned address;
    int size;
    // Reading lines like " M 20,1" or "L 19,3"
    while (fscanf(pFile, " %c %x,%d\n", &identifier, &address, &size) >= 0)
    {
        //得到标记位和组序号
        int op_tag = address >> (s + b);
        int op_s = (address >> b) & (((unsigned)(-1)) >> (8 * sizeof(unsigned) - s));
        switch (identifier)
        {
        case 'M': //修改（一次存储一次加载）
            update_info(op_tag, op_s);
            update_info(op_tag, op_s);
            break;
        case 'L'://存储
            update_info(op_tag, op_s);
            break;
        case 'S'://加载
            update_info(op_tag, op_s);
            break;
        }
        update_time_stamp();
    }
    fclose(pFile);
}

void print_help()
{
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("-h         Print this help message.\n");
    printf("-v         Optional verbose flag.\n");
    printf("-s <num>   Number of set index bits.\n");
    printf("-E <num>   Number of lines per set.\n");
    printf("-b <num>   Number of block offset bits.\n");
    printf("-t <file>  Trace file.\n\n\n");
    printf("Examples:\n");
    printf("linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}
int main(int argc, char *argv[])
{
    char opt;
    int s, E, b;
    /*
     * s:S=2^s是组的个数
     * E:每组中有多少行
     * b:B=2^b每个缓冲块的字节数
     */
    while (-1 != (opt = getopt(argc, argv, "hvs:E:b:t:")))
    {
        switch (opt)
        {
        case 'h':
            print_help();
            exit(0);
        case 'v':
            verbose = 1;
            break;
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
            strcpy(t, optarg);
            break;
        default:
            print_help();
            exit(-1);
        }
    }
    if(s<=0 || E<=0 || b<=0 || t==NULL) // 如果选项参数不合格就退出
	        return -1;
    Init_Cache(s, E, b); //初始化一个cache
    get_trace(s, E, b);
    free_Cache();
    // printSummary(hit_count, miss_count, eviction_count)
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}