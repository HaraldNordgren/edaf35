#define _BSD_SOURCE

#define	N			32
//#define POOL_SIZE	16 * 1024 * 1024
#define POOL_SIZE	((size_t) 1 << N)

#include <unistd.h>
#include <stddef.h>

void* calloc(size_t nmemb, size_t size);
void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);

typedef struct list_t list_t;
struct list_t {
	unsigned	reserved:1;
	char		kval;
	list_t*		succ;
	list_t*		pred;
	char		data[];
};

void	init_pool();
char	getkval(size_t s);
size_t	getpow2sz(char k);
list_t*	merge(list_t* p);

static list_t*	freelist[N] = { NULL };
static list_t*	pool = NULL;

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
	size_t	tot_sz;
	char	kval;
	int		i;
	list_t*	p;
	list_t*	q;
	list_t*	r;

	// initialize pool
	if (pool == NULL) {
		init_pool();
	}

	tot_sz = sizeof(list_t) + size;
	kval = getkval(tot_sz);	

	// find smallest free block that fits
	i = kval - 1;
	while (i < N && (p = freelist[i]) == NULL) i++;

	if (p == NULL)
		return NULL;

	// split node
	while (i > kval - 1) {
		// remove from list
		if (p->succ == p) {
			freelist[i] = NULL;
		} else {
			p->pred->succ = p->succ;
			p->succ->pred = p->pred;
			freelist[i] = p->succ;
		}

		// reduce size
		p->kval = i;

		// initiate buddy
		q = (list_t*) ((char*) p + (1 << i));
		q->reserved = 0;
		q->kval = i;
		q->succ = p;
		q->pred = p;

		p->succ = q;
		p->pred = q;

		// insert into list
		i--;
		r = freelist[i];
		if (r != NULL) {
			q->succ = r;
			p->pred = r->pred;
			r->pred->succ = p;
			r->pred = q;
		}
		freelist[i] = p;
	}

	// remove from list
	if (p->succ == p) {
		freelist[i] = NULL;
	} else {
		p->pred->succ = p->succ;
		p->succ->pred = p->pred;
		freelist[i] = p->succ;
	}

	p->reserved = 1;
	p->pred = p;
	p->succ = p;

	return p->data;
}


void free(void* ptr)
{
	list_t*	p;
	list_t*	q;
	int		i;

	if (ptr == NULL)
		return;

	p = (list_t*) ((char*) ptr - sizeof(list_t));
	
	// merge blocks
	p = merge(p);
	p->reserved = 0;

	i = p->kval - 1;

	// insert into list
	q = freelist[i];
	if (q != NULL) {
		p->pred = q->pred;
		p->succ = q;
		q->pred->succ = p;
		q->pred = p;
	}

	freelist[i] = p;
}

void* realloc(void* ptr, size_t size)
{
	void*	mem;
	list_t*	p;
	list_t*	q;
	size_t	pow2_sz ;
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

		pow2_sz = getpow2sz(q->kval);	
		min = pow2_sz - sizeof(list_t) < size ? pow2_sz - sizeof(list_t) : size;

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

//////////////////////////////// HELPER FUNCTIONS ////////////////////////////////

void init_pool()
{
	pool = sbrk(POOL_SIZE);
	pool->reserved = 0;
	pool->kval = N;
	pool->succ = pool;
	pool->pred = pool;
	freelist[N - 1] = pool;
}

char getkval(size_t s)
{
	size_t	pow2_sz;
	char	kval;
	
	pow2_sz = 1;
	kval = 0;
	while (pow2_sz < s) {
		pow2_sz <<= 1;
		kval++;
	}

	return kval;
}

size_t getpow2sz(char k)
{
	size_t	pow2_sz;
	char	kval;
	
	pow2_sz = 1;
	kval = 0;
	while (kval < k) {
		pow2_sz <<= 1;
		kval++;
	}

	return pow2_sz;
}

list_t* merge(list_t* p) 
{
	list_t*	q;
	int		i;

	q = (list_t*) ((char*) pool + (((char*) p - (char*) pool) ^ (1 << p->kval)));
	
	if (p->kval < N && !q->reserved && q->kval == p->kval) {
		i = p->kval - 1;

		// remove buddy from list
		if (q->succ == q) {
			freelist[i] = NULL;
		} else {
			q->pred->succ = q->succ;
			q->succ->pred = q->pred;
			freelist[i] = q->succ;
		}
	
		// take first block
		p = p < q ? p : q;

		// initiate block
		p->kval++;
		p->pred = p;
		p->succ = p;
		
		// recurse
		p = merge(p);
	}

	return p;
}

