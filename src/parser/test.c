#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "lexer.h"

int main(int argc, char **argv)
{
    int result;
    int i;
    const char *filename = "test.c";
    token_array_t array;
    error_info_t error_info;

    if (argc > 1) {
        filename = argv[1];
    }

    if ((result=token_init()) != 0) {
        return result;
    }

    //coment 1  
    //comment 2

    printf("d=%lld, =x\x31=\n", 07LL);
    memset( &error_info, 0, sizeof(error_info));
    if ((result=lexer_parse(filename, &array, &error_info)) == 0) {
    } else {
        fprintf(stderr, "file: %s, %d:%d, %s\n",
                error_info.position.filename, error_info.position.lineno,
                error_info.position.colno, error_info.error);
    }

    printf("count: %d\n", array.count);
    /*
    for (i=0; i<array.count; i++) {
        printf("id: %d, line %d:%d, %.*s(%d)\n",
        array.items[i].info->id,
        array.items[i].position.lineno, 
        array.items[i].position.colno,
        array.items[i].str.len, array.items[i].str.str,
        array.items[i].str.len);
    }
    */

    token_destroy();
    return 0;
}
