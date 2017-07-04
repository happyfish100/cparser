#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "lexer.h"

int main(int argc, char **argv)
{
    int result;
    const char *filename = "test.c";
    token_array_t array;
    error_info_t error_info;

    if (argc > 1) {
        filename = argv[1];
    }

    if ((result=token_init()) != 0) {
        return result;
    }

    if ((result=lexer_parse(filename, &array, &error_info)) == 0) {
    } else {
        fprintf(stderr, "%s\n", error_info.error);
    }

    token_destroy();
    return 0;
}
