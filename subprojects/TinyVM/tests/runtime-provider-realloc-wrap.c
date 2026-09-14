#include <stddef.h>

void *__wrap_malloc(size_t size){
    (void)size;
    return NULL;
}

void *__wrap_realloc(void *pointer,size_t size){
    (void)pointer;
    (void)size;
    return NULL;
}
