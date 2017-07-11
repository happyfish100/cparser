#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fastcommon/hash.h>
#include "token.h"

static token_info_t tokens[] = {
#define T(mode, id, tag, memo, val, is_keyword) \
    {is_keyword, id, {tag, sizeof(tag) - 1}, memo},
#include "token_define.h"
#undef T
    {false, T_NULL, {NULL, 0}, "NULL"}
};

static HashArray token_hash_array;
int g_max_operator_len = 0;
special_tokens_t g_special_tokens;

#define TOKEN_SET(t, mode, ID, desc, is_key) \
    do { \
       t.is_keyword = is_key; \
       t.id = ID;             \
       t.token.str = NULL;    \
       t.token.len = 0;       \
       t.memo = desc;         \
    } while (0)

static void init_special_tokens()
{
    TOKEN_SET(g_special_tokens.preprocessor, _ALL, T_PREPROCESSOR, "preprocessor", false);
    TOKEN_SET(g_special_tokens.backslash, _ALL, T_BACKSLASH, "backslash", false);
    TOKEN_SET(g_special_tokens.newline, _ALL, T_NEWLINE, "new line", false);
    TOKEN_SET(g_special_tokens.eof, _ALL, T_EOF, "end of input", false);
    TOKEN_SET(g_special_tokens.identifier, _ALL, T_IDENTIFIER, "identifier", false);
    TOKEN_SET(g_special_tokens.number, _ALL, T_NUMBER, "number constant", false);
    TOKEN_SET(g_special_tokens.character, _ALL, T_CHARACTER_CONSTANT, "character constant", false);
    TOKEN_SET(g_special_tokens.string, _ALL, T_STRING_LITERAL, "string literal", false);
    TOKEN_SET(g_special_tokens.line_comment,  _ALL, T_LINE_COMMENT, "line comment as // ...", false);
    TOKEN_SET(g_special_tokens.block_comment, _ALL, T_BLOCK_COMMENT, "block comment as /* ... */", false);
}

int token_init()
{
    int result;
    int token_count;
    int i;

    token_count = sizeof(tokens) / sizeof(token_info_t);
    if ((result=hash_init(&token_hash_array, simple_hash, token_count * 5, 0.00)) != 0) {
        return result;
    }
    for (i=0; i<token_count; i++) {
        if (tokens[i].token.str != NULL) {
            if (!tokens[i].is_keyword) {
                if (tokens[i].token.len > g_max_operator_len) {
                    g_max_operator_len = tokens[i].token.len;
                }
            }
            if ((result=hash_insert_ex(&token_hash_array, tokens[i].token.str,
                            tokens[i].token.len, tokens + i, 0, false)) != 1)
            {
                return ENOMEM;
            }
            printf("%d. %.*s => %s\n", tokens[i].id, tokens[i].token.len, tokens[i].token.str, tokens[i].memo);
        }
    }
    hash_stat_print(&token_hash_array);
    printf("token_count: %d, g_max_operator_len: %d\n", token_count, g_max_operator_len);

    init_special_tokens();
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
