#include <stddef.h>

void *__wrap_realloc(void *pointer,size_t size){
    (void)pointer;
    (void)size;
    return NULL;
}
