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
    token_id_t id;
    string_t token;
    const char *memo;
} token_info_t;

#ifdef __cplusplus
extern "C" {
#endif

    int token_init();
    void token_destroy();
    token_info_t *token_find(char *str, const int len);

#ifdef __cplusplus
}
#endif

#endif