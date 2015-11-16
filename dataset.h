#ifndef _DATASET_H_
#define _DATASET_H_

struct dataset_struct;
typedef struct dataset_struct * dataset_t;

extern int dataset_create(dataset_t* dataset);
extern int dataset_destroy(dataset_t* dataset);
extern int dataset_add(dataset_t collection, void* data);
extern void* dataset_get(dataset_t collection, char* key);
extern void* dataset_iter(dataset_t collection, void** iterator);

#endif
