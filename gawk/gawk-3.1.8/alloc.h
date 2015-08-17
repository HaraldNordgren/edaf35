#ifndef ALLOC_H
#define ALLOC_H

#define DEBUG_1     1
#define N           (24)
#define POOL_SIZE   (1L << N)

#define VOID(ptr)   ((void*) (ptr))

typedef struct list_t list_t;

struct list_t {
    //unsigned    reserved:1;
    char        reserved;
    char        kval;
    list_t*     pred;
    list_t*     succ;
    char        data[];
};

#define LIST_T sizeof(list_t)

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

void print_freelists();
void print_memory();

#endif
