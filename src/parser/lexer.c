#include "fastcommon/shared_func.h"
#include "lexer.h"

static int do_parse(char *content, const int file_size,
        token_array_t *array, error_info_t *error_info)
{
    return 0;
}

int lexer_parse(const char *filename, token_array_t *array,
        error_info_t *error_info)
{
    int result;
    char *content;
    int64_t file_size;

    if ((result=getFileContent(filename, &content, &file_size)) != 0)
    {
        snprintf(error_info->error, MAX_ERROR_SIZE,
                "read from file %s fail", filename);
        return result;
    }

    result = do_parse(content, file_size, array, error_info);
    free(content);
    return result;
}
