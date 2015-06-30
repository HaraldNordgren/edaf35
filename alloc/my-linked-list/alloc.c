#define MY_MALLOC 1

#if !(MY_MALLOC)
    #include <stdlib.h>
#endif

#include <stdio.h>
#include <unistd.h>

#define SIZE    (1024)
#define N       (8 * SIZE*SIZE / sizeof(size_t))

typedef struct list_t list_t;

struct list_t {
    size_t  size;
    list_t  *next;
    char    data[];
};

#define LIST_T sizeof(list_t)
#define SIZE_T sizeof(size_t)

static list_t *avail = NULL;
static size_t *memory_start;

void print_memory() {
    size_t i;

    printf("\n");

#if 1
    for (i = 0; i < 10; i++) {
        printf("%p:\t%016zx\n", memory_start + i, memory_start[i]);
    }
#endif

#if 0
    for (i = 0; i < 0x10; i++) {
        char *nbr = (char*) memory_start + i;
        printf("%p:\t%016x\n", nbr, nbr[i]);
    }
#endif

}

#if MY_MALLOC
void *malloc(size_t size) {
    list_t *prev, *p, *mem_segment;
    size_t *new, min_size;
    
    if (size <= 0) {
        return NULL; 
    }

    prev = NULL;
    p = avail;

    min_size = size + LIST_T;

    while (p != NULL && p->size < min_size) {
        prev = p;
        p = p->next;
    }

    if (p == NULL) {
        mem_segment = sbrk(min_size);
        mem_segment->size = min_size;
        
        return mem_segment->data;
    }
    
    if (p->size >= min_size + LIST_T + 8) {
        printf("large segment\n");
        
        /*list_t list_entry = *p;

        new = (size_t*) p;
        *new = min_size;
        

        list_entry.size = list_entry.size - min_size;
        list_entry.addr = list_entry.addr + min_size;

        if (prev != NULL) {
            prev->next = (list_t*) list_entry.addr;
        }

        return new + 1;*/
    }
     
    if (prev != NULL) {
        printf("\nprev:\t\t%p\n", prev);
        printf("prev->next:\t%p\n", prev->next);
        printf("\np:\t\t%p\n", p);
        printf("p->next:\t%p\n\n", p->next);
        
        prev->next = p->next;
    }
    
    if (p == avail) {
        printf("\nmalloc:\n");
        printf("avail:\t\t%p\t\n", avail);
        printf("avail->next:\t%p\n", avail->next);

        avail = avail->next;
    }

    return p->data;
}

void free(void *ptr) {
    list_t  *p, *prev, list_entry, *mem_segment;
    size_t  size;
    char    *addr;

    mem_segment = (list_t*) ( (char*) ptr - LIST_T );

    if (avail == NULL) {
        avail = mem_segment;
        
        printf("\nfree:\n");
        printf("avail:\t\t%p\n", avail);
        printf("avail->next:\t%p\n", avail->next);
        
        return;
    }

    prev = NULL;
    p = avail;
    
    while (p != NULL && p < mem_segment) {
        prev = p;
        p = p->next;
    }

    if (prev == NULL) {
        list_t *tmp = avail;

        avail = p;
        p->next = tmp;
        
        return;
    }


    if (p == NULL) {
        printf("\nhej\n");
        prev->next = mem_segment;
        return;
    }
 
    mem_segment->next = p;
    prev->next = mem_segment;
}
#endif

int main() {

    memory_start = sbrk(0);

#if 1
    size_t *p, *q;
    
    printf("before first malloc\n");
    printf(" p:\t%p\n", p);
    
    p = malloc(8*10);
    
    printf("\nafter first malloc\n");
    printf(" p:\t%p\n", p);
    print_memory();
    
    printf("\nafter filling p\n");
    *p = 0x1122;
    print_memory();
    
    
    free(p);
    printf("\nafter free\n");
    print_memory();
    
#if 1
    p = malloc(3);
    *p = 0x3344;
    
    printf("\nafter second malloc and filling p\n");
    printf(" p:\t%p\n", p);
    print_memory();
#endif

    q = malloc(2);
    printf("\nafter third malloc\n");
    printf(" q:\t%p\n", q);
    print_memory();
    
    q = malloc(2);
    printf("\nafter fourth malloc\n");
    printf(" q:\t%p\n", q);
    print_memory();
    
    q = malloc(2);
    printf("\nafter fifth malloc\n");
    printf(" q:\t%p\n", q);
    print_memory();
#endif

    return 0;
}
