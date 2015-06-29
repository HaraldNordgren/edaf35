/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */

char *
test_malloc(int size)
{
  return (char *)1;
}

void
test_free(char *ptr)
{
}

char *
test_realloc(char *ptr, int size)
{
  return ptr;
}
