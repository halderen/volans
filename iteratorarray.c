#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ldns/ldns.h>
#include "proto.h"

struct names_iterator_struct {
    int (*iterate)(names_iterator*iter, void**);
    int (*advance)(names_iterator*iter, void**);
    int (*end)(names_iterator*iter);
    int count;
    int index;
    void** array;
};

static int
iterateimpl(names_iterator*i, void** item)
{
    struct names_iterator_struct** iter = i;
    if (item)
        *item = NULL;
    if (*iter) {
        if ((*iter)->index < (*iter)->count) {
            if (item)
                *item = (*iter)->array[(*iter)->index];
            return 1;
        } else {
            free((*iter)->array);
            free(*iter);
            *iter = NULL;
        }
    }
    return 0;
}

static int
advanceimpl(names_iterator*i, void** item)
{
    struct names_iterator_struct** iter = i;
    if(*iter) {
        if((*iter)->index+1 < (*iter)->count) {
            (*iter)->index += 1;
            if(item)
                *item = (*iter)->array[(*iter)->index];
            return 1;
        }
        free((*iter)->array);
        free(*iter);
        *iter = NULL;
    }
    return 0;
}

static int
endimpl(names_iterator*iter)
{
    if(*iter) {
        free((*iter)->array);
        free(*iter);
    }
    *iter = NULL;
    return 0;
}

names_iterator
names_iterator_array(int count, void* base, size_t memsize, size_t offset)
{
    struct names_iterator_struct* iter;
    void** array;
    int i;
    array = malloc(sizeof(void*) * count);
    for(i=0; i<count; i++) {
        array[i] = *(char**)&(((char*)base)[memsize*i+offset]);
        assert(array[i]);
    }
    iter = malloc(sizeof(struct names_iterator_struct));
    iter->iterate = iterateimpl;
    iter->advance = advanceimpl;
    iter->end = endimpl;
    iter->count = count;
    iter->index = 0;
    iter->array = array;
    return iter;
}

names_iterator
names_iterator_array2(int count, void* base, size_t memsize)
{
    struct names_iterator_struct* iter;
    void** array;
    int i;
    array = malloc(sizeof(void*) * count);
    for(i=0; i<count; i++) {
        array[i] = &(((char*)base)[memsize*i]);
        assert(array[i]);
    }
    iter = malloc(sizeof(struct names_iterator_struct));
    iter->iterate = iterateimpl;
    iter->advance = advanceimpl;
    iter->end = endimpl;
    iter->count = count;
    iter->index = 0;
    iter->array = array;
    return iter;
}
