#ifndef IPJ_SHMEM_H
#define IPJ_SHMEM_H

#include <stddef.h>
#include <stdbool.h>

int ipj_shm_init(void);
void ipj_shm_cleanup(void);

// TODO: Support anonymous objects
void *ipj_shalloc(
	// [in]
	const char *name,

	// [inout, nullable]
	size_t *size,

	// [out, nullable]
	bool *is_new);

void ipj_shfree(void *shm);

#endif
