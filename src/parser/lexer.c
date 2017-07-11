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

#define IS_C_SPACE(ch) (ch == ' ' || ch == '\t' || ch == '\r')

#define MOVE_PTR_NEXT(context) \
    do { \
        p++;              \
        context->position.colno++; \
    } while (0)

#define MOVE_PTR(context, n) \
    do { \
        p += n;              \
        context->position.colno += n; \
    } while (0)

#define INC_LINE_NO(context) \
    do { \
        context->position.lineno++;  \
        context->position.colno = 0; \
    } while (0)

#define MOVE_PTR_NEXT_AND_INC_LINE_NO(context) \
    do { \
        p++;              \
        context->position.lineno++;  \
        context->position.colno = 0; \
    } while (0)

#define SET_ERROR(context, str) \
    do { \
        context->error_info->position = context->position;               \
        snprintf(context->error_info->error, MAX_ERROR_SIZE, "%s", str); \
    } while (0)

#define SET_ERROR_WITH_PARAM1(context, format, p1) \
    do { \
        context->error_info->position = context->position;                \
        snprintf(context->error_info->error, MAX_ERROR_SIZE, format, p1); \
    } while (0)

#define SET_ERROR_WITH_PARAM2(context, format, p1, p2) \
    do { \
        context->error_info->position = context->position;                    \
        snprintf(context->error_info->error, MAX_ERROR_SIZE, format, p1, p2); \
    } while (0)

#define SET_WARNING(context, str) \
    do { \
        if (context->error_info->strict) { \
            SET_ERROR(context, str);       \
            return EINVAL;  \
        } else {            \
            fprintf(stderr, "%s:%d:%d, warning: %s\n", \
                    context->position.filename,        \
                    context->position.lineno,          \
                    context->position.colno, str);     \
        }  \
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

    MOVE_PTR_NEXT(context);
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
        SET_ERROR_WITH_PARAM1(context, "expect end character: %c", end_char);
        return EINVAL;
    }

    MOVE_PTR_NEXT(context);
    return 0;
}

static int parse_line_comment(lexer_context_t *context, token_entry_t *token)
{
    char *pr;
    int line_count = 0;

    MOVE_PTR(context, 2);
    while (1) {
        while ((p < context->end) && (*p != '\n')) {
            MOVE_PTR_NEXT(context);
        }

        line_count++;
        if (p == context->end) {
            break;
        }

        pr = p - 1;
        while (IS_C_SPACE(*pr)) {
            pr--;
        }

        if (*pr == '\\') {
            MOVE_PTR_NEXT_AND_INC_LINE_NO(context);  //skip \n
        } else {
            break;
        }
    }

    printf("p==%.*s, line %d:%d\n", 10, p, context->position.lineno, context->position.colno);
    if (line_count > 1) {
        SET_WARNING(context, "multi-line comment");
    }

    return 0;
}

static int parse_block_comment(lexer_context_t *context, token_entry_t *token)
{
    MOVE_PTR(context, 2);
    while ((p + 1 < context->end) && !((*p == '*') && *(p+1) == '/')) {
        if (*p == '\n') {
            MOVE_PTR_NEXT_AND_INC_LINE_NO(context);
        } else {
            MOVE_PTR_NEXT(context);
        }
    }

    if (p + 1 == context->end) {
        SET_ERROR(context, "expect end of block comment");
        return EINVAL;
    }

    MOVE_PTR(context, 2);
    return 0;
}

static int parse_number_part(lexer_context_t *context, token_entry_t *token,
        bool *is_integer)
{
    bool have_point;
    if (*p == '.') {
        have_point = true;
        MOVE_PTR(context, 2);  //skip .#
    } else if ((*p == '0') && (p + 1 < context->end)) {
        if (*(p+1) == 'x' || *(p+1) == 'X') { //hex char 0xHH
            MOVE_PTR(context, 2);  //skip 0x
            if (!(p < context->end && IS_HEX_CHAR(*p))) {
                SET_ERROR_WITH_PARAM1(context, "invalid hex character: %c", *p);
                return EINVAL;
            }

            do {
                MOVE_PTR_NEXT(context);
            } while (p < context->end && IS_HEX_CHAR(*p));
            *is_integer = true;
            return 0;

        } else {
            have_point = false;
            MOVE_PTR_NEXT(context);
        }
    } else {
        have_point = false;
        MOVE_PTR_NEXT(context);
    }

    while (p < context->end) {
        if (*p == '.') {
            if (have_point) {
                break;
            }
            have_point = true;
            MOVE_PTR_NEXT(context);
        } else if (IS_DIGIT_CHAR(*p)) {
            MOVE_PTR_NEXT(context);
        } else {
            break;
        }
    }

    *is_integer = !have_point;
    return 0;
}

static int parse_number_suffix(lexer_context_t *context, token_entry_t *token,
        const bool is_integer)
{
    if (!((p < context->end) && IS_LETTER_CHAR(*p))) {
        return 0;
    }

    if (is_integer) {
        if (*p == 'u' || *p == 'U') {
            MOVE_PTR_NEXT(context);
            if (!((p < context->end) && IS_LETTER_CHAR(*p))) {
                return 0;
            }
            if (*p == 'l' || *p == 'L') {
                MOVE_PTR_NEXT(context);
            } else {
                SET_ERROR_WITH_PARAM2(context, "invalid suffix \"%c%c\" "
                        "on integer constant", *(p-1), *p);
                return EINVAL;
            }
        } if (*p == 'l' || *p == 'L') {
            MOVE_PTR_NEXT(context);
            if (!((p < context->end) && IS_LETTER_CHAR(*p))) {
                return 0;
            }
            if (*p == 'u' || *p == 'U') {
                MOVE_PTR_NEXT(context);
            } else if ((*p == 'l' || *p == 'L') && (*(p - 1) == *p)) { //LL or ll
                MOVE_PTR_NEXT(context);
            } else {
                SET_ERROR_WITH_PARAM2(context, "invalid suffix \"%c%c\" "
                        "on integer constant", *(p-1), *p);
                return EINVAL;
            }
        } else {
            SET_ERROR_WITH_PARAM1(context, "invalid suffix \"%c\" "
                    "on integer constant", *p);
            return EINVAL;
        }
    } else {
        if ((*p == 'f' || *p == 'F') || (*p == 'l' || *p == 'L')) {
            MOVE_PTR_NEXT(context);
        } else {
            SET_ERROR_WITH_PARAM1(context, "invalid suffix \"%c\" "
                    "on float constant", *p);
            return EINVAL;
        }
    }

    return 0;
}

static int parse_number(lexer_context_t *context, token_entry_t *token)
{
    int result;
    bool is_integer;
    char *start;

    start = p;
    if ((result=parse_number_part(context, token, &is_integer)) != 0) {
        return result;
    }

    if ((p + 1 < context->end) && (*p == 'e')) {  //scientific notation
        if (!IS_DIGIT_CHAR(*(p+1))) {
            SET_ERROR(context, "exponent has no digits");
            return EINVAL;
        }
        MOVE_PTR(context, 2);  //skip e#
        while (p < context->end && IS_DIGIT_CHAR(*p)) {
            MOVE_PTR_NEXT(context);
        }
        is_integer = false;
    } else if (is_integer && ((*start == '0') && ((start + 1) < p)
                && IS_DIGIT_CHAR(*(start+1))))
    { //octal number
        start++;   //skip 0
        while (start < p && IS_DIGIT_CHAR(*start)) {
            if (!IS_OCT_CHAR(*start)) {
                SET_ERROR(context, "invalid octal digits");
                return EINVAL;
            }
            start++;
        }
    }
           
    return parse_number_suffix(context, token, is_integer);
}

static void parse_identifier(lexer_context_t *context, token_entry_t *token)
{
    while (p < context->end && ((*p == '_') || IS_LETTER_CHAR(*p)
                || IS_DIGIT_CHAR(*p)))
    {
        MOVE_PTR_NEXT(context);
    }
}

static int parse_preprocessor(lexer_context_t *context, token_entry_t *token)
{
    char *pr;

    MOVE_PTR_NEXT(context);
    while (1) {
        while ((p < context->end) && (*p != '\n')) {
            MOVE_PTR_NEXT(context);
        }

        if (p == context->end) {
            break;
        }

        pr = p - 1;
        while (IS_C_SPACE(*pr)) {
            pr--;
        }

        if (*pr == '\\') {
            MOVE_PTR_NEXT_AND_INC_LINE_NO(context);  //skip \n
        } else {
            break;
        }
    }

    return 0;
}

static int parse_backslash(lexer_context_t *context, token_entry_t *token)
{
    int space_count;
    MOVE_PTR_NEXT(context);
    if ((p < context->end) && (*p == '\n')) {
        return 0;
    }

    space_count = 0;
    while ((p < context->end) && (IS_C_SPACE(*p) && *p != '\n')) {
        MOVE_PTR_NEXT(context);
        space_count++;
    }

    if (p < context->end) {
        if (*p != '\n') {
            SET_ERROR(context, "stray '\\' in program\n");
            return EINVAL;
        }
    } else {
        SET_WARNING(context, "backslash-newline at end of file");
    }

    if (space_count > 0) {
        SET_WARNING(context, "backslash and newline separated by spaces");
    }

    return 0;
}

static int parse_operator(lexer_context_t *context, token_entry_t *token)
{
    token_info_t *t;
    token_info_t *last_token;
    int len;
    int last_len;

    last_len = len = 0;
    last_token = NULL;
    do {
        len++;
        t = token_find(p, len);
        if (t != NULL) {
            last_token = t;
            last_len = len;
        }
    } while (p + len < context->end && len < g_max_operator_len);

    if (last_token != NULL) {
        token->info = last_token;
        MOVE_PTR(context, last_len);
        return 0;
    } else {
        SET_ERROR_WITH_PARAM1(context, "unkown character %c", *p);
        return EINVAL;
    }
}

static int parse_token(lexer_context_t *context, token_entry_t *token)
{
    int result;
    int char_count;

    result = 0;
    switch (*p) {
        case '\n':
            token->info = &g_special_tokens.newline;

            MOVE_PTR_NEXT_AND_INC_LINE_NO(context);  //skip \n
            return 0;
        case '\'':  //character
            token->info = &g_special_tokens.character;
            result = parse_string(context, token, *p, &char_count);
            if (result == 0 && char_count != 1) {
                SET_ERROR_WITH_PARAM1(context, "expect one character, "
                        "but %d characters occur", char_count);
                result = EINVAL;
            }
            return result;
        case '\"':  //string
            token->info = &g_special_tokens.string;
            return parse_string(context, token, *p, &char_count);
        case '/':  //maybe comment
            if (p + 1 < context->end) {
                if (*(p + 1) == '/') {
                    token->info = &g_special_tokens.line_comment;
                    return parse_line_comment(context, token);
                } else if (*(p + 1) == '*') {
                    token->info = &g_special_tokens.block_comment;
                    return parse_block_comment(context, token);
                }
            }
            break;
        case '.':  //maybe number
            if ((p + 1 < context->end) && IS_DIGIT_CHAR(*(p+1))) {
                token->info = &g_special_tokens.number;
                return parse_number(context, token);
            }
            break;
        case '#':
            token->info = &g_special_tokens.preprocessor;
            return parse_preprocessor(context, token);
        case '\\':
            token->info = &g_special_tokens.backslash;
            return parse_backslash(context, token);
        default:
            if (IS_DIGIT_CHAR(*p)) {  //number
                token->info = &g_special_tokens.number;
                return parse_number(context, token);
            } else if ((*p == '_') || IS_LETTER_CHAR(*p)) { //identifier
                token->info = &g_special_tokens.identifier;
                MOVE_PTR_NEXT(context);
                parse_identifier(context, token);
                return 0;
            }
            break;
    }

    return parse_operator(context, token);
}

static int next_token(lexer_context_t *context, token_entry_t *token)
{
    int result;

    while ((p < context->end) && IS_C_SPACE(*p)) {
        MOVE_PTR_NEXT(context);
    }

    if (p == context->end) {
        return ENOENT;
    }

    token->position = context->position;
    token->str.str = p;

    result = parse_token(context, token);
    token->str.len = p - token->str.str;

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

        token->info = NULL;
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
    context.position.lineno = 1;
    context.start = content;
    context.end = content + file_size;
    context.cur = content;

    array->items = NULL;
    array->size = array->count = 0;

    result = do_parse(&context, array);
    //free(content);
    return result;
}
