#include "malloc.h"
#include <stdio.h>

/* This benchmark allocates a lot of items of the same size
 * and then deallocates them in another order and sometimes
 * allocats new items.
 */
typedef struct list
{
	struct list * prev;
	struct list * next;
	double data;
} list;

int main()
{
	void *start, *end;
	int i;
	list * base;
	list * newitem;
	start = sbrk(0);

	base = malloc(sizeof(list));
	base->prev = base;
	base->next = base;
	base->data = 0;

	/* fill the list */
	for (i = 0; i < 100000; ++i)
	{
		/* create a new item and add it to the list */
		newitem = malloc(sizeof(list));
		newitem->prev = base;
		newitem->next = base->next;
		base->next = newitem;
		newitem->next->prev = newitem;
	}

	i = 0;
	/* reduce the list */
	while (base->next != base)
	{
		/* Remove an item from the list */
		base->prev->next = base->next;
		base->next->prev = base->prev;
		newitem = base;
		base = base->next->next; /* skip over one */
		free(newitem);
		newitem = 0;

		++i;
		if (i == 3)
		{
			i = 0;
			/* create a new item and add it to the list */
			newitem = malloc(sizeof(list));
			newitem->prev = base;
			newitem->next = base->next;
			base->next = newitem;
			newitem->next->prev = newitem;
		}

	}
	free(base);
	end = sbrk(0);
	printf("using %ld bytes\n", (unsigned long)(end-start));
}
