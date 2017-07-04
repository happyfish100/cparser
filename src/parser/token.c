#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fastcommon/hash.h>
#include "token.h"

static token_info_t tokens[] = {
#define T(mode, name, tag, memo, val, is_keyword) {name, {tag, sizeof(tag) - 1}, memo},
#include "token_define.h"
#undef T
};

static HashArray token_hash_array;

int token_init()
{
    int result;
    int token_count;
    int i;

    token_count = sizeof(tokens) / sizeof(token_info_t);
    printf("token_count: %d\n", token_count);

    if ((result=hash_init(&token_hash_array, simple_hash, 1024, 0.00)) != 0) {
        return result;
    }
    for (i=0; i<token_count; i++) {
        if (tokens[i].token.str != NULL) {
            if ((result=hash_insert_ex(&token_hash_array, tokens[i].token.str,
                            tokens[i].token.len, tokens + i, 0, false)) != 1)
            {
                return ENOMEM;
            }
            printf("%d. %.*s => %s\n", tokens[i].id, tokens[i].token.len, tokens[i].token.str, tokens[i].memo);
        }
    }
    hash_stat_print(&token_hash_array);
    return 0;
}

void token_destroy()
{
    hash_destroy(&token_hash_array);
}

token_info_t *token_find(char *str, const int len)
{
    return (token_info_t *)hash_find(&token_hash_array, str, len);
}
