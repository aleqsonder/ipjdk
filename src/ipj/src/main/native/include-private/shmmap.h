#ifndef IPJ_SHMMAP_H
#define IPJ_SHMMAP_H

#include <stddef.h>
#include <stdbool.h>

typedef struct ipj_shm ipj_shm_t;

ipj_shm_t *ipj_shm_open(const char *name, size_t size, bool *is_new);
void ipj_shm_close(ipj_shm_t *);
void *ipj_shm_data(const ipj_shm_t *);

#endif
