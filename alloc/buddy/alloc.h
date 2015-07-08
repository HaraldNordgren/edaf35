#ifndef ALLOC_H
#define ALLOC_H

#define N           (22)
#define POOL_SIZE   (1 << N)

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

void print_freelists();

typedef struct list_t list_t;

struct list_t {
    unsigned    reserved:1;
    char        kval;
    list_t*     succ;
    list_t*     pred;
    char        data[];
};

#define LIST_T sizeof(list_t)
#define SIZE_T sizeof(size_t)

#endif
