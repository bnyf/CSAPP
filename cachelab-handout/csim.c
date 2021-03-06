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
int EVICTION;//是否替换
void init(){
    S = E = B = 0;
    memset(fileName,0,sizeof(fileName));
    h = v = 0;
    hit_count = miss_count = eviction_count = 0;
}

struct Cache_line{
    int vis; //有效
    unsigned long long flag; //标记
};
struct Cache_line *cache_line;

struct LRU_node{
    struct LRU_node *next;
    int idx;//组内行的下标
};
struct LRU_node **lru_root_node;

int find_cacheline(unsigned long long flag, int set_idx){
    int line_idx_start = set_idx * E;
    int line_idx_end = line_idx_start + E;
    for(int i=line_idx_start;i<line_idx_end;++i){
        if(cache_line[i].vis > 0){
            if(cache_line[i].flag == flag){
                hit_count++;
                HIT = 1;
                struct LRU_node *p = lru_root_node[set_idx];
                
                //在LRU中去掉该命中节点
                while(p->next){
                    struct LRU_node *temp = p->next;
                    if((temp->idx) == (i % E)){
                        temp = temp->next;
                        free(p->next);
                        p->next = temp;
                        return i % E;
                    }
                    p = p->next;
                }
		//printf("error\n");
            }
        }
    }

    miss_count++;

    for(int i=line_idx_start;i<line_idx_end;++i){
        if(cache_line[i].vis == 0){
            return i % E; //返回组内空行下标
        }
    }

    return replace_cacheline(set_idx); //未命中并返回替换后的组内空行下标
}

//返回替换掉的 cacheline 的组内行数，并更新 LRU
int replace_cacheline(int set_idx){
    // printf(" eviction_count");
    eviction_count++;
    EVICTION = 1;
    struct LRU_node *p = lru_root_node[set_idx];
    struct LRU_node *temp = p->next;
    p->next = temp->next;
    int line_idx_in_set = temp->idx;
    free(temp);
    
    return line_idx_in_set;
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
    free(lru_root_node);
}

//使用该 cacheline，并在 LRU 中进行更新
void use_cacheline(unsigned long long flag, int line_idx_in_set, int set_idx){
    // printf("begin use cacheline\n");
    int line_idx_in_cache = line_idx_in_set + set_idx * E;
    cache_line[line_idx_in_cache].vis = 1; 
    cache_line[line_idx_in_cache].flag = flag;

    struct LRU_node *p = lru_root_node[set_idx];
    // printf("begin append LRU node\n");
    // printf("idx:%d\n",p->idx);
    while(p->next){
        p = p->next;
    }
    // printf("finish append\n");
    p->next = malloc(sizeof(struct LRU_node));
    struct LRU_node *temp = p->next;
    temp->next = NULL;
    temp->idx = line_idx_in_set;
    // printf("end use cacheline\n");
}

void cache(char c, unsigned long long addr, int size){
    HIT = 0;
    EVICTION = 0;
    int set_idx = (addr / B) % S;
    unsigned long long flag = addr / B / S;
    int line_idx_in_set = find_cacheline(flag,set_idx);//获取本次存取在cache中的行数
    // printf("set_idx:%d, flag:%d, line_idx_in_set:%d, hit:%d, v:%d\n",set_idx, flag, line_idx_in_set, HIT, v);
    if(v == 1){
         printf("%c %llx,%d",c,addr,size);
    }
    switch(c){
        case 'M':
            if(v == 1){

                if(HIT == 0)
                    printf(" miss");
                else
                    printf(" hit"); 

                if(EVICTION == 1)
                printf(" eviction");
                    printf(" hit\n");
            }

            use_cacheline(flag, line_idx_in_set, set_idx);
            hit_count++;
        break;
        case 'L':
        case 'S':
	    if(v == 1){
            	if(HIT == 0)
                   printf(" miss");
            	else
                   printf(" hit"); 
 	         
		if(EVICTION == 1)
		   printf(" eviction");
	        
		printf("\n"); 
	    }
    
            use_cacheline(flag, line_idx_in_set, set_idx);
            break;
	default:
	    break;
    }
}

int main(int argc, char * argv[])
{
    // printf("start main func\n");
    init();
    char ch;
    int s; //set bits
    int b; //block bits 
    // printf("start getopt\n");
    while((ch = getopt(argc, argv ,"hvs:E:b:t:")) != EOF){
        switch(ch) {
            case 'h':
                h = 1;
                break;
            case 'v':
		// printf("v = 1\n");
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
		// printf("t = 1\n");
                memcpy(fileName,optarg,strlen(optarg)+1);
		//printf("finish copy filename\n");
            	break;
	    case '?':
		break;
        }
    }
    // printf("h:%d v:%d s:%d E:%d b:%d t:%s\n",h,v,s,E,b,fileName);
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
    // printf("S:%d, B:%d\n",S,B);
    line_num = S * E;
    cache_line = malloc(sizeof(struct Cache_line) * line_num);
    lru_root_node = malloc(sizeof(struct LRU_node*) * S);
    for(int i=0;i<S;++i){
    	lru_root_node[i] = malloc(sizeof(struct LRU_node*));
    }

    char c[2];
    unsigned long long addr;
    int size;
    FILE* stream = fopen(fileName, "r");
//    printf("begin read from %s\n",fileName);
    while(fscanf(stream," %s %llx,%d",c,&addr,&size) != EOF){
	// printf("c:%s, addr:%x, size:%d\n",c,addr,size);
	if(c[0] != 'I')
        	cache(c[0], addr, size);
    }
    clear();
    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
