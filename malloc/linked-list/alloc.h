#ifndef ALLOC_H
#define ALLOC_H

#define DEBUG 1

#if 0
#define SIZE    (1024)
#define N       (8 * SIZE*SIZE / sizeof(size_t))
#endif

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

typedef struct list_t list_t;

struct list_t {
    size_t  size;
    list_t  *next;
    char    data[];
};

#define LIST_T sizeof(list_t)
#define SIZE_T sizeof(size_t)

list_t* get_avail(void);
void print_avail();
void print_memory();

#endif
