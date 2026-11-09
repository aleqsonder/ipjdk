#ifndef IPJ_SHMEM_H
#define IPJ_SHMEM_H

#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>

int ipj_shm_init(void);
void ipj_shm_cleanup(void);

ssize_t ipj_shm_alloc(size_t size);

ssize_t ipj_shm_provide(
	// [in]
	const char *name,

	// [inout, nullable]
	size_t *size,

	// [out, nullable]
	bool *is_new);

void ipj_shm_release(ssize_t handle);

void *ipj_shm_handle_resolve(ssize_t handle);

#endif
