#include "fastcommon/shared_func.h"
#include "lexer.h"

typedef struct lexer_context_t {
    char *start;
    char *end;
    char *cur;  //current
    position_t position;
    error_info_t *error_info;
} lexer_context_t;

#define p (context->cur)

#define MOVE_PTR_NEXT(context) \
    do { \
        p++;              \
        context->position.colno++; \
    } while (0)

#define SET_ERROR(context, str) \
    do { \
        context->error_info->position = context->position;               \
        snprintf(context->error_info->error, MAX_ERROR_SIZE, "%s", str); \
    } while (0)

static int parse_escaped_char(lexer_context_t *context)
{
    if (p == context->end) {
        SET_ERROR(context, "expect escape character after \\");
        return EINVAL;
    }

    switch (*p) {
        case 'a':
        case 'b':
        case 'f':
        case 'n':
        case 'r':
        case 't':
        case 'v':
        case '\\':
        case '?':
        case '\'':
        case '"':
        case '0':
            break;
        case 'x':  //hex as \xhh
            MOVE_PTR_NEXT(context);
            if (p == context->end) {
                SET_ERROR(context, "expect hex characters after \\x");
                return EINVAL;
            }
            if (!IS_HEX_CHAR(*p)) {
                SET_ERROR(context, "expect hex characters after \\x");
                return EINVAL;
            }

            if ((p < context->end) && IS_HEX_CHAR(*(p+1))) {
                MOVE_PTR_NEXT(context);
            }
            break;
        default:
            if (!IS_OCT_CHAR(*p)) {
                SET_ERROR(context, "invalid characters after \\");
                return EINVAL;
            }
            if ((p < context->end) && IS_OCT_CHAR(*(p+1))) {
                MOVE_PTR_NEXT(context);
            }
            if ((p < context->end) && IS_OCT_CHAR(*(p+1))) {
                MOVE_PTR_NEXT(context);
            }
            break;
    }

    MOVE_PTR_NEXT(context);
    return 0;
}

static int parse_string(lexer_context_t *context, token_entry_t *token,
        const char end_char, int *char_count)
{
    int result;
    result = 0;
    *char_count = 0;
    while ((p < context->end) && (*p != end_char)) {
        if (*p == '\n') {
            result = EINVAL;
            break;
        }

        (*char_count)++;
        MOVE_PTR_NEXT(context);
        if (*(p - 1) == '\\') {  //escape
            if ((result=parse_escaped_char(context)) != 0) {
                return result;
            }
        }
    }

    if (result != 0 || p == context->end) {
        context->error_info->position = context->position;
        snprintf(context->error_info->error, MAX_ERROR_SIZE,
                "expect end character: %c", end_char);
        return EINVAL;
    }

    return 0;
}

static int next_token(lexer_context_t *context, token_entry_t *token)
{
    int result;
    int char_count;

    while ((p < context->end) && (*p == ' ' || *p == '\t' || *p == '\r')) {
        MOVE_PTR_NEXT(context);
    }

    if (p == context->end) {
        return ENOENT;
    }

    token->str.str = p;
    switch (*p) {
        case '\n':
            token->position = context->position;
            token->info = &g_special_tokens.newline;

            MOVE_PTR_NEXT(context);
            context->position.lineno++;
            context->position.colno = 0;
            break;
        case '\'':  //character
        case '\"':  //string
            MOVE_PTR_NEXT(context);
            result = parse_string(context, token, *p, &char_count);
            break;
        case '/':  //maybe comment
            token->info = &g_special_tokens.line_comment;
            token->info = &g_special_tokens.block_comment;
            break;
        case '.':  //maybe number
            if ((p < context->end) && (*(p+1) >= '0' && *(p+1) <= '9')) {
                p++;
            }
            break;
        default:
            if ((*p == '_') || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) {
            } else if (*p >= '0' && *p <= '9') {
            }
            break;
    }

    token->str.len = 1;   //TODO

    return result;
}

static int get_next_token(lexer_context_t *context, token_entry_t *token)
{
    int result;

    result = next_token(context, token);
    if (result != 0) {
        if (result == ENOENT) {
            token->position = context->position;
            token->info = &g_special_tokens.eof;
            return 0;
        }
        return result;
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
            context->error_info->position = context->position;
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
    } while (p < context->end);
    return result;
}

int lexer_parse(const char *filename, token_array_t *array,
        error_info_t *error_info)
{
    int result;
    char *content;
    int64_t file_size;
    lexer_context_t context;

    context.position.filename = filename;
    context.position.lineno = 0;
    context.position.colno = 0;
    context.error_info = error_info;

    if ((result=getFileContent(filename, &content, &file_size)) != 0)
    {
        error_info->position = context.position;
        snprintf(error_info->error, MAX_ERROR_SIZE,
                "read from file %s fail", filename);
        return result;
    }
    context.start = content;
    context.end = content + file_size;
    context.cur = content;

    array->items = NULL;
    array->size = array->count = 0;

    result = do_parse(&context, array);
    free(content);
    return result;
}
