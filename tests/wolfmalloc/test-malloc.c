#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
    uint8_t* mallocme, *mallocme2, *mallocme3;
    fprintf(stdout, "-- malloc test by Wolfgang Richter <wolf@cs.cmu.edu> --"
                    "\n");

    mallocme = malloc(4096);
    mallocme2 = malloc(4096);
    mallocme3 = malloc(4096);

    memset(mallocme, 0, 4096);
    memset(mallocme2, 0xff, 4096);
    memset(mallocme3, 0xee, 4096);

    free(mallocme);
    free(mallocme2);
    free(mallocme3);

    mallocme = malloc(4096);
    mallocme2 = malloc(4096);
    mallocme3 = malloc(4096);

    memset(mallocme, 0, 4096);
    memset(mallocme2, 0xff, 4096);
    memset(mallocme3, 0xee, 4096);

    free(mallocme3);
    free(mallocme2);
    free(mallocme);

    printf("success\n");


    return EXIT_SUCCESS;
}
