#include <search.h>
#include <errno.h>

struct dataset_struct;

#define KEY 0xf000
#define STRUCTURE 0x0f00
#define dataset_int16 0x1000
//#define dataset_int32 0x2000
//#define dataset_int64 0x3000
#define dataset_chararray 0x4000
#define dataset_charptr   0x5000
#define dataset_HASHTABLE 0x0100
#define dataset_TREE      0x0200
#define dataset_LIST      0x0300
#define dataset_NONE      0X0400

typedef struct dataset_struct * dataset_t;

extern int dataset_create(dataset_t* dataset);
extern int dataset_destroy(dataset_t* dataset);
extern int dataset_add(dataset_t collection, void* data);
extern void* dataset_get(dataset_t collection, char* key);
extern void* dataset_iter(dataset_t collection, void** iterator);

struct record {
    ENTRY entry;
    char key[12];
    struct record* next;
    struct record* prev;
};

/*
int32_t
32bits 6 bits per char 5bytes+2bits
64bits 6 bits per char 10bytes+4bits
 */

struct dataset_struct {
    struct record* first;
    struct record* last;
    int style;
};

int dataset_create(dataset_t* dataset, int type) {
    if (!dataset) {
        return EINVAL;
    }
    if (!(*dataset = malloc(sizeof (struct dataset_struct)))) {
        return ENOMEM;
    }
    (*dataset)->first = NULL; 
   (*dataset)->last = NULL;
    (*dataset)->style = style;
    switch (style) {
        case dataset_NONE:
            break;
        case dataset_LIST:
            break;
        case dataset_TREE:
        case dataset_HASHTABLE:
    }
    return 0;
}

int dataset_destroy(dataset_t* dataset) {
    if (dataset) {
        if ((*dataset)->first) {
            return -3;
        }
        if (free(*dataset))
            return EINVAL;
    } else {
        return -2;
    }
    return 0;
}

int dataset_add(dataset_t collection, void* data) {
    struct record* record;
    if (!data || !collection) {
        return -2;
    }
    if (!(record = malloc(sizeof (struct record)))) {
        return -1;
    }
    record->entry->key = data;
    record->entry->data = data;
    record->next = NULL;
    if (collection->first) {
        collection->last->next = record;
    }
}

void* dataset_get(dataset_t collection, char* key) {
    switch (collection->type) {
        case dataset_NONE:
            return NULL;
        case dataset_HASHTABLE;
    }
}

void* dataset_iter(dataset_t collection, void** iterator);
