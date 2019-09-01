#include "cachelab.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#define MAX_FILENAME_LENGTH 4096

int replace_cacheline(int set_idx);
extern char *optarg;
extern int optind, opterr, optopt;
int hit_count, miss_count, eviction_count;

int S,E,B;
char fileName[MAX_FILENAME_LENGTH];
int h,v;
int line_num; //行总数
int HIT; //是否命中

void init(){
    S = E = B = 0;
    memset(fileName,0,sizeof(fileName));
    h = v = 0;
    hit_count = miss_count = eviction_count = 0;
}

struct Cache_line{
    int vis; //有效
    int flag; //标记
};
struct Cache_line *cache_line;

struct LRU_node{
    struct LRU_node *next;
    int idx;//组内行的下标
};
struct LRU_node **lru_root_node;
int *LRU_node_num;

int find_cacheline(int flag, int set_idx){
    int line_idx_start = set_idx * E;
    int line_idx_end = line_idx_start + E;
    for(int i=line_idx_start;i<line_idx_end;++i){
        if(HIT == 1)
            break;
        if(cache_line[i].vis > 0){
            if(cache_line[i].flag == flag){
                hit_count++;
                HIT = 1;
                struct LRU_node *p = lru_root_node[set_idx];
                while(p->next){
                    struct LRU_node *temp = p->next;
                    if(temp->idx == (i % E)){
                        temp = temp->next;
                        free(p->next);
                        p->next = temp;
                        p = p->next;
                        break;
                    }
                    p = p->next;
                }
                cache_line[i].vis = 0;
                cache_line[i].flag = 0;
            }
        }
    }
    if(HIT == 0){
        miss_count++;
    }

    for(int i=line_idx_start;i<line_idx_end;++i){
        if(!cache_line[i].vis){
            return i % E; //返回组内空行下标
        }
    }

    return replace_cacheline(set_idx); //未命中并返回替换后的组内空行下标
}

int replace_cacheline(int set_idx){
    printf(" eviction_count");
    eviction_count++;
    struct LRU_node *p = lru_root_node[set_idx];
    struct LRU_node *temp = p->next;
    p->next = temp->next;
    int res = temp->idx;
    free(temp);
    p->next = NULL;
    return res;
}

void clear(){
    for(int i=0;i<S;++i){
        struct LRU_node *p = lru_root_node[i];
        struct LRU_node *temp = p;
        while(p){
            p = p->next;
            free(temp);
            temp = p;
        }
    }
    free(cache_line);
    free(lru_root_node)
}
void use_cacheline(int flag, int line_idx_in_set, int set_idx){
    int line_idx_in_cache = line_idx_in_set + set_idx * E;
    cache_line[line_idx_in_cache].vis = 1; 
    cache_line[line_idx_in_cache].flag = flag;

    struct LRU_node *p = lru_root_node[set_idx];
    while(p->next){
        p = p->next;
    }
    p->next = malloc(sizeof(struct LRU_node));
    struct LRU_node *temp = p->next;
    temp->next = NULL;
    temp->idx = line_idx_in_set;
}

void cache(char c, int addr){
    int set_idx = addr / B % S;
    int flag = addr / B / S;
    int line_idx_in_set = find_cacheline(flag,set_idx);
    HIT = 0;
    if(v == 1){
        printf("%c %x,%d ",c,addr,size);
    }
    switch(c){
        case 'I':
            break;
        case 'M':
            if(HIT == 0)
                printf(" miss");
            else
                printf(" hit"); 

            use_cacheline(flag, line_idx_in_set, set_idx);
                
            printf("hit\n");
            hit_count++;
            break;
        case 'L':
            if(HIT == 0)
                printf(" miss");
            else
                printf(" hit"); 
 
            use_cacheline(flag, line_idx_in_set, set_idx);

            printf("\n");
            break;
        case 'S':
            if(HIT == 0)
                printf(" miss");
            else
                printf(" hit"); 
 
            use_cacheline(flag, line_idx_in_set, set_idx);

            printf("\n");
            break;
    }
}

printSummary(hit_count, miss_count, eviction_count){
    pritnf("hits:%d misses:%d evictions:%d\n",hit_count,miss_count,eviction_count);
}

int main(int argc, char * argv[])
{
    init();
    char ch;
    int s; //set bits
    int b; //block bits 
    while((ch = getopt(argc, argv ,"hvs:E:b:t:"))){
        switch(ch) {
            case 'h':
                h = 1;
                break;
            case 'v':
                v = 1;
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
                memcpy(fileName,optarg,sizeof(optarg)+1);
                break;
        }
    }
    if(h == 1){
        printf("-h: Optional help flag that prints usage info\n" \
        "-v: Optional verbose flag that displays trace info\n" \
        "-s <s>: Number of set index bits (S = 2s is the number of sets)\n" \
        "-E <E>: Associativity (number of lines per set)\n" \
        "-b <b>: Number of block bits (B = 2b is the block size)\n" \
        "-t <tracefile>: Name of the valgrind trace to replay\n");
    }
    S = pow(2,s);
    B = pow(2,b);
    line_num = S * E;
    cache_line = malloc(sizeof(struct Cache_line) * line_num);
    lru_root_node = malloc(sizeof(struct LRU_node*) * S);

    char c[2];
    unsigned int addr,size;
    FILE* stream = fopen(fileName, "w+");
    while(fscanf(stream,"%s %x,%d",c,&addr,&size) != EOF){
        cache(c[0], addr);
    }
    clear();
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
