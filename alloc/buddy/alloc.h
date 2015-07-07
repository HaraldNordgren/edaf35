#ifndef ALLOC_H
#define ALLOC_H

#define N       (9)
#define SIZE    (1 << N)

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

typedef struct list_t list_t;

struct list_t {
    unsigned    reserved:1;
    char        kval;
    list_t*     succ;
    list_t*     pred;
};

#if 0
static list_t* freelist[10];
#endif

#define LIST_T sizeof(list_t)
#define SIZE_T sizeof(size_t)

#endif
