// Tomer Elmalem
#include "my_malloc.h"
#include <string.h>
#include <stdio.h>

int main() {
        char *a;
        char b[] = "hello peepz";
        printf("String of size %d:\n",strlen(b));
        a = my_malloc(sizeof(char)*strlen(b));
        strncpy(a, b, strlen(b));

        printf("%s\n", a);
        printf("%s\n", b);

        my_free(a);

        char *c;
        char d[] = "iz thiz workingz?";
        printf("String of size %d:\n",strlen(b));
        c = my_malloc(sizeof(char)*strlen(d));
        strncpy(c, d, strlen(d));

        printf("%s\n", c);
        printf("%s\n", d);

        char e[] = "10 Home\n20 Sweet\n30 GOTO 10";
        printf("String of size %d:\n",strlen(e));
        c = my_realloc(c, sizeof(char)*strlen(e));
        strncpy(c, e, strlen(e));

        printf("%s\n", c);
        printf("%s\n", e);

        // Free everything
        my_free(c);

	return 0;
}
