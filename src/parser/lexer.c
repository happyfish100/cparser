#include "fastcommon/shared_func.h"
#include "lexer.h"

typedef struct lexer_context_t {
    char *start;
    char *end;
    char *current;
    int lineno;
    int colno;
    error_info_t *error_info;
} lexer_context_t;

static int next_token(lexer_context_t *context, string_t *token)
{
    return 0;
}

static int get_next_token(lexer_context_t *context, token_entry_t *token)
{
    token->position.filename = NULL;   //TODO
    token->position.lineno = context->lineno;
    token->position.colno = context->colno;
    if (context->current == context->end) {
        token->info = &g_special_tokens.eof;
        return 0;
    }

    return 0;
}

static token_entry_t *alloc_token(token_array_t *array)
{
    if (array->size <= array->count) {
        int bytes;
        if (array->size == 0) {
            array->size = 1024;
        } else {
            array->size *= 2;
        }

        bytes = sizeof(token_entry_t) * array->size;
        array->items = (token_entry_t *)realloc(array->items, bytes);
        if (array->items == NULL) {
            fprintf(stderr, "malloc %d bytes fail", bytes);
            return NULL;
        }
    }

    return array->items + array->count;
}

static int do_parse(lexer_context_t *context, token_array_t *array)
{
    int result;
    token_entry_t *token;

    do {
        if ((token=alloc_token(array)) == NULL) {
            snprintf(context->error_info->error, MAX_ERROR_SIZE,
                    "alloc token entry fail");
            result = ENOMEM;
            break;
        }

        if ((result=get_next_token(context, token)) == 0) {
            array->count++;
        } else {
            break;
        }
    } while (context->current < context->end);
    return result;
}

int lexer_parse(const char *filename, token_array_t *array,
        error_info_t *error_info)
{
    int result;
    char *content;
    int64_t file_size;
    lexer_context_t context;

    if ((result=getFileContent(filename, &content, &file_size)) != 0)
    {
        snprintf(error_info->error, MAX_ERROR_SIZE,
                "read from file %s fail", filename);
        return result;
    }

    context.start = content;
    context.end = content + file_size;
    context.current = content;
    context.lineno = 0;
    context.colno = 0;
    context.error_info = error_info;

    array->items = NULL;
    array->size = array->count = 0;

    result = do_parse(&context, array);
    free(content);
    return result;
}
