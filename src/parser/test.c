#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"

int main(int argc, char **argv)
{
    int result;
    if ((result=token_init()) != 0) {
        return result;
    }
    token_destroy();
    return 0;
}
