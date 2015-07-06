#ifndef ALLOC_H
#define ALLOC_H

#define SIZE    (1024)
#define N       (8 * SIZE*SIZE / sizeof(size_t))

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

static list_t *avail = NULL;

#ifdef DEBUG
list_t* get_avail() {
    return avail;
}
#endif

#endif
