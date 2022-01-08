#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct cacheline {
    int valid;
    int dirty;
    int tag;
    int touch;
} CacheLine;

void initialCache(CacheLine *Cache, int BlockNumber);
// void replacement(CacheLine *Cache, int replace_mode, int associativity, int index, int tag, int *cnt);

int main(int argc, char *argv[]) {
    if(argc < 6) {
        fprintf(stderr, "Invalid Input Format: ./cache <cache_size> <block_size> <associativity> <replace_policy> <file>\n");
        exit(0);
    }
    
    int CacheSize = 0, BlockSize = 0, BlockNumber = 0, associativity = 0, replace_mode = 0;
    char *filename = NULL;

    filename = argv[5] + 2;
    FILE *fp = fopen(filename, "r");

    if(fp == NULL){
        perror("Open file error");
        exit(0);
    }

    CacheSize = atoi(argv[1]) * 1024;
    BlockSize = atoi(argv[2]);
    BlockNumber = CacheSize / BlockSize;
    
    CacheLine *Cache = (CacheLine *) malloc(sizeof(CacheLine) * BlockNumber);

    if(strncmp("f", argv[3], strlen(argv[3])) != 0) {
        associativity = atoi(argv[3]);
    }
    else {
        associativity = BlockNumber;    //  Fully associativit
    }

    if(strncmp("LRU", argv[4], strlen(argv[4])) == 0)   //  Default mode is FIFO
        replace_mode = 1;   

    int mode = 0, Address = 0, readWord = 0, index = 0, tag = 0, cacheHit = 0, cacheMiss = 0, cnt = 0, time = 0, Instruction = 0, readData = 0, wrtieData = 0, fromMem = 0, toMem = 0;
    char buf[1024];

    initialCache(Cache, BlockNumber);   // Initialize valid bit is zero
    while(fgets(buf, 1024, fp) != NULL) {
        readWord = sscanf(buf, "%d %x", &mode, &Address);

        if(readWord < 2) {
            fprintf(stderr, "Data format error: %s", buf);
            exit(0);
        }
        if(Address == 0xffffffff)
            continue;

        if(mode == 0)
            readData++;
        else if(mode == 1)
            wrtieData++;

        Address = Address / BlockSize;  // Remove offset
        index = Address % (BlockNumber / associativity);
        tag = Address / (BlockNumber / associativity);
       
        int pos = 0, needReplace = 1;

        for(int i=0; i < associativity; i++) {
            pos = index*associativity + i;
            if(Cache[pos].tag == tag && Cache[pos].valid != 0) {
                cacheHit++;     
                Cache[pos].touch = (replace_mode == 0)? Cache[pos].touch: (++cnt); 
                Cache[pos].dirty = (mode == 1)? 1: Cache[pos].dirty;
                needReplace = 0; 
                break;
            }          
        }
        if(needReplace != 0) {
            int minIndex = index*associativity;

            cacheMiss++;
            for(int i=0; i<associativity; i++) {
                if(Cache[index*associativity + i].valid == 0) {     // Compulsory miss
                    minIndex = index*associativity + i;
                    Cache[minIndex].valid = 1;
                    break;
                } else {
                    minIndex = (Cache[index*associativity + i].touch < Cache[minIndex].touch)? (index*associativity + i): minIndex;
                }
            }
            fromMem += BlockSize;
            toMem += (Cache[minIndex].dirty)? BlockSize: 0;
            Cache[minIndex].touch = (++cnt);
            Cache[minIndex].tag = tag;
            Cache[minIndex].dirty = (mode == 1)? 1: 0;
        }
    }
   
    for(int i=0; i<BlockNumber; i++) {
        toMem += (Cache[i].dirty)? BlockSize: 0;
    }

    Instruction = cacheMiss + cacheHit;
    printf("Input file = %s\n", filename);
    printf("Demand fetch = %d\n", Instruction);
    printf("Cache hit = %d\n", cacheHit);
    printf("Cache miss = %d\n", cacheMiss);
    printf("Miss rate = %.4lf\n", (double)cacheMiss/Instruction);
    printf("Read data = %d\n", readData);
    printf("Write data = %d\n", wrtieData);
    printf("Bytes from memory = %d\n", fromMem);
    printf("Byte to memory = %d\n", toMem);
    free(Cache);
    fclose(fp);
    
    return 0;
}

void initialCache(CacheLine *Cache, int BlockNumber) {
    for(int i=0; i<BlockNumber; i++) {
        Cache[i].valid = 0;
        Cache[i].dirty = 0;
    }
}

// void replacement(CacheLine *Cache, int replace_mode, int associativity, int index, int tag, int *cnt) {
//     int minIndex = index;
//     for(int i=1; i<associativity; i++) {
//         minIndex = (Cache[index+i].touch < Cache[minIndex].touch)? (index+i): minIndex;
//     }
//     Cache[minIndex].tag = tag;
//     Cache[minIndex].touch = (replace_mode == 0)? (++*cnt): 1;
// }