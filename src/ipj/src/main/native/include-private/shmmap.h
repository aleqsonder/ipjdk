#ifndef IPJ_SHMMAP_H
#define IPJ_SHMMAP_H

#include <stddef.h>

void *shmopen(const char *name, size_t size);
void shmclose(void *shm);

#endif
