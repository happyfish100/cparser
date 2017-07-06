#ifndef _TOKEN_H
#define _TOKEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fastcommon/common_define.h>

typedef enum token_id_t {
        T_NULL  =  0,
#define T(mode, x, tag, memo, val, is_keyword) x val,
#include "token_define.h"
#undef T
        T_LAST_TOKEN
} token_id_t;

typedef enum pp_token_id_t {
        TP_NULL = 0,
#define T(token) TP_##token,
#include "tokens_preprocessor.h"
#undef T
        TP_LAST_TOKEN
} pp_token_id_t;

typedef struct token_info_t {
    bool is_keyword;
    token_id_t id;
    string_t token;
    const char *memo;
} token_info_t;

typedef struct special_tokens_t {
    token_info_t newline;
    token_info_t eof;
    token_info_t line_comment;   //comment as: // ...
    token_info_t block_comment;  //comment as: /* ... */
    token_info_t identifier;
    token_info_t number;
    token_info_t character;
    token_info_t string;
} special_tokens_t;

#ifdef __cplusplus
extern "C" {
#endif

    extern int g_max_operator_len;
    extern special_tokens_t g_special_tokens;
    int token_init();
    void token_destroy();
    token_info_t *token_find(char *str, const int len);

#ifdef __cplusplus
}
#endif

#endif
