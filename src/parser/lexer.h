#ifndef _LEXER_H
#define _LEXER_H

#include "types.h"
#include "token.h"

typedef struct token_entry_t {
    token_info_t *info;
    position_t position;
    string_t str;
} token_entry_t;

typedef struct token_array_t {
    token_entry_t *items;
    int count;   //element count
    int size;    //alloc size
} token_array_t;

#ifdef __cplusplus
extern "C" {
#endif

    int lexer_parse(const char *filename, token_array_t *array,
            error_info_t *error_info);

#ifdef __cplusplus
}
#endif

#endif
