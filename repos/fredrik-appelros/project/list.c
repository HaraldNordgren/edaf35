#define _BSD_SOURCE

#define	MINIMUM_DATA_SIZE	4

#include <unistd.h>
#include "alloc.h"

typedef struct list_t list_t;
struct list_t {
	size_t	size;
	list_t*	next;
	char	data[];
};

static list_t avail = { .next = NULL };

void* calloc(size_t nmemb, size_t size)
{
	void*	mem;
	char*	bytes;
	size_t	tot_sz;
	size_t	i;
   
	tot_sz = nmemb * size;

	mem = malloc(tot_sz);

	// clear data
	if (mem != NULL) {
		bytes = mem;
		for (i = 0; i < tot_sz; i++) {
			bytes[i] = 0;
		}
	}

	return mem;
}

void* malloc(size_t size)
{
	void*	mem;
	list_t*	p;
	list_t*	q;
	list_t*	r;
	size_t tot_sz;
	
	if (size == 0)
		return NULL;

	tot_sz = sizeof(list_t) + size;

	// search for free space
	p = &avail;
	q = avail.next;
	while (q != NULL && q->size < tot_sz) {
		p = q;
		q = q->next;
	}

	if (q == NULL) {
		// no free space; allocate new
		mem = sbrk(tot_sz);

		// no memory left; abort
		if (mem == (void*) -1)
			return NULL;

		// set size of new node
		q = (list_t*) mem;
		q->size = tot_sz;
	} else {
		// reusing old memory; check if it should be split
		if (q->size >= tot_sz + sizeof(list_t) + MINIMUM_DATA_SIZE) {
			// old node should be split
			r = (list_t*) ((char*) q + tot_sz);

			// set sizes of nodes
			r->size = q->size - tot_sz;
			q->size = tot_sz;
			
			// insert split node into list
			p->next = r;
			r->next = q->next;
		} else {
			// remove old node from list
			p->next = q->next;
		}
	}

	return q->data;
}

void free(void* ptr)
{
	list_t*	p;
	list_t*	q;
	list_t*	r;
	int		merge_l;
	int		merge_r;
	
	if (ptr == NULL)
		return;

	r = (list_t*) ((char*) ptr - sizeof(list_t));

	// search for place for r in list
	p = &avail;
	q = avail.next;
	while (q != NULL && r > q) {
		p = q;
		q = q->next;
	}
	
	// check if nodes can be merged
	merge_l = (list_t*) ((char*) p + p->size) == r;
	merge_r = (list_t*) ((char*) r + r->size) == q;

	if (merge_l && merge_r) {
		p->size += r->size + q->size;
		p->next = q->next;
	} else if (merge_l) {
		p->size += r->size;
	} else if (merge_r) {
		r->size += q->size;
		p->next = r;
		r->next = q->next;
	} else {
		p->next = r;
		r->next = q;
	}
}

void* realloc(void* ptr, size_t size)
{
	void*	mem;
	list_t*	p;
	list_t*	q;
	size_t	min;
	size_t	i;

	mem = malloc(size);

	// no memory left; abort
	if (mem == NULL && size > 0)
		return NULL;

	if (mem != NULL) {
		p = (list_t*) ((char*) mem - sizeof(list_t));
	}

	if (ptr != NULL) {
		q = (list_t*) ((char*) ptr - sizeof(list_t));
		
		min = q->size - sizeof(list_t) < size ? q->size - sizeof(list_t) : size;

		// copy data
		for (i = 0; i < min; i++) {
			p->data[i] = q->data[i];
		}
		
		free(ptr);
	}

	if (mem == NULL)
		return NULL;

	return p->data;
}

