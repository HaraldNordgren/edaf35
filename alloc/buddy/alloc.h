#ifndef ALLOC_H
#define ALLOC_H

#define DEBUG_1 1

#define N           (32)
//#define N           (12)
#define POOL_SIZE   ((size_t) 1 << N)

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

void print_freelists();
void print_memory();

typedef struct list_t list_t;

struct list_t {
    //unsigned    reserved:1;
    char        reserved;
    char        kval;
    list_t*     succ;
    list_t*     pred;
    char        data[];
};

#define LIST_T sizeof(list_t)
#define SIZE_T sizeof(size_t)

#endif
