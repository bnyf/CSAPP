#include "cachelab.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define MAX_FILENAME_LENGTH 4096
int hit_count, miss_count, eviction_count;

int S,E,B;
char fileName[MAX_FILENAME_LENGTH];
bool h,v;
int line_num;

void init(){
    s = E = b = 0;
    memeset(fileName,0,sizeof(fileName));
    h = v = false;
    hit_count = miss_count = eviction_count = 0;
}

struct Cache_line{
    bool vis;
    int flag;
};
struct Cache_line *cache_line;

struct LRU_node{
    struct LRU_node *next;
    int idx;
};
struct LRU_node **lru_node;
int *LRU_node_num;
int find_idle_cacheline(int flag, int set_idx){
    int line_idx_start = set_idx * K;
    int line_idx_end = line_idx_start + K;
    for(int i=line_idx_start;i<line_idx_end;++i){
        if(cache_line[i].vis){
            if(cache_line[i].flag == flag){
                hit_count++;
                return -1;
            }
        }
    }
    miss_count++;
    for(int i=line_idx_start;i<line_idx_end;++i){
        if(!cache_line[i].vis){
            return i % K;
        }
    }

    return replace_cacheline(set_idx);
}

int replace_cacheline(int set_idx){
    printf("eviction_count ");
    eviction_count++;
    LRU_node *p = lru_node[set_idx];
    struct LRU_node *res = p->next;
    p->next = res->next;
    while(p->next->next != NULL){
        p = p->next;
    }
    delete(p->next);
    p->next = NULL;
    return temp->idx;
}

void use_cacheline(int flag, int line_idx_in_set, int set_idx){
    int line_idx_in_cache = line_idx_in_set + set_idx * E;
    cache_line[line_idx_in_cache].vis = true;
    cache_line[line_idx_in_cache].flag = flag;

    struct LRU_node *p = lru_node[set_index];
    while(p->next){
        p = p->next;
    }
    p->next = malloc(sizeof(struct LRU_node));
    struct LRU_node temp = p->next;
    temp.next = NULL;
    temp.idx = line_idx_in_set;
}

void cache(char c, int addr, int size){
    int set_idx = addr / B % S;
    int flag = addr / B /S;
    int line_idx_in_set = find_idle_cacheline(flag,set_idx);
    if(v == 1){
        printf("%c %x,%d ",c,addr,size);
    }
    switch(c){
        case 'I':
            break;
        case 'M':
            if(line_idx_in_set > 0){
                printf(" miss");
                use_cacheline(flag, line_idx_in_set, set_idx);
            }
            else{
                printf(" hit"); 
            }
            printf("hit\n");
            hit_count++;
            break;
        case 'L':
            if(line_idx_in_set > 0){
                printf(" miss");   
                use_cacheline(flag, line_idx_in_set, set_idx);
            }
            else{
                printf(" hit");
            }
            printf("\n");

            break;
        case 'S':
            if(line_idx_in_set > 0){
                printf(" miss");
                use_cacheline(flag, line_idx_in_set, set_idx);
            }
            else{
                printf(" hit");
            }
            printf("\n");

            break;
    }
}

int main(int argc, char * argv[])
{
    init();

    char ch;
    int s,b;
    while((ch = getopt(argc, argv ,"hvs:E:b:t:"))){
        witch(ch) {
            case 'h':
                h = true;
                break;
            case 'v':
                v = true;
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
        "-t <tracefile>: Name of the valgrind trace to replay\n"
    }
    S = pow(2,s);
    B = pow(2,b);
    line_num = S * B;
    cache_line = malloc(sizeof(struct Cache_line) * line_num);
    lru_node = malloc(sizeof(struct LRU_node*) * S);
    lru_node_num = malloc(sizeof(int) * S);
    char c[2];
    int addr,size;
    while(fscanf(fileName,"%s %x,%d",c,addr,size) != EOF){
        cache(c, addr, size);
    }
    delete(sizeof(struct Cache_line) * line_num);

    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}