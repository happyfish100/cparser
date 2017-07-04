#ifndef _TYPES_H
#define _TYPES_H

#include <fastcommon/common_define.h>

#define MAX_ERROR_SIZE 256

typedef struct position_t {
    const char *filename;
    int lineno;
    int colno;
} position_t;

typedef struct error_info_t {
    position_t position;
    char error[MAX_ERROR_SIZE];
} error_info_t;

#endif
