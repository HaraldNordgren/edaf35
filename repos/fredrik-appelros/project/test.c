#include <assert.h>
#include <stdio.h>
#include "alloc.h"

#define	ARRAY_SIZE	10

int main()
{
	int*	i_ptr;
	int*	j_ptr;
	int*	k_ptr;
	int*	l_ptr;
	int*	m_ptr;
	size_t	i;

	i_ptr	= (int*) malloc(sizeof(int));
	j_ptr	= (int*) malloc(sizeof(int));
	k_ptr	= (int*) malloc(sizeof(int));
	*i_ptr = 1;
	*j_ptr = 2;
	*k_ptr = 3;

	assert(*i_ptr == 1);
	assert(*j_ptr == 2);
	assert(*k_ptr == 3);

	free(i_ptr);
	l_ptr	= (int*) malloc(sizeof(int));
	*l_ptr = 4;

	assert(*l_ptr == 4);
	assert(*i_ptr == 4);

	free(l_ptr);
	free(k_ptr);
	free(j_ptr);

	l_ptr	= (int*) malloc(sizeof(int));
	m_ptr	= (int*) malloc(sizeof(int));
	*l_ptr = 5;
	*m_ptr = 6;
	
	assert(*l_ptr == 5);
	assert(*m_ptr == 6);

	i_ptr	= (int*) calloc(ARRAY_SIZE, sizeof(int));
	
	for (i = 0; i < ARRAY_SIZE; i++) {
		assert(i_ptr[i] == 0);
		i_ptr[i] = i + 1;
	}

	i_ptr	= (int*) realloc(i_ptr, ARRAY_SIZE / 2 * sizeof(int));
	
	for (i = 0; i < ARRAY_SIZE / 2; i++) {
		assert(i_ptr[i] == i + 1);
	}

	free(l_ptr);
	i_ptr	= (int*) realloc(i_ptr, sizeof(int));
	
	assert(*i_ptr == 1);

	i_ptr	= (int*) realloc(i_ptr, 2 * ARRAY_SIZE * sizeof(int));

	assert(i_ptr[0] == 1);

	puts("ALL TESTS PASSED");

	return 0;
}
