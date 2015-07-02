#include <stdlib.h>
void *firstFitAllocRegion(size_t s);
void freeRegion(void *r);
void *resizeRegion(void *r, size_t newSize);
