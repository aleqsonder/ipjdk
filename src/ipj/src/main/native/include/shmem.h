#ifndef IPJ_SHMEM_H
#define IPJ_SHMEM_H

#include <stddef.h>

enum ipj_shmerror {
	IPJ_SHM_SUCCESS = 0,
	
	// unknown error
	// (probably check for errno on POSIX or GetLastError on Windows)
	IPJ_SHM_UNKNOWN = 1,

	// no memory available
	IPJ_SHM_NOMEM,

	// memory object with that name does exist
	// but it has less space than required
	IPJ_SHM_EXIST_LESS
};

enum ipj_shmerror ipj_shm_init(void);
void ipj_shm_destroy(void);

enum ipj_shmerror ipj_shalloc(
	const char *name,
	size_t min_size,
	void **mem);

void ipj_shfree(void *mem);

#endif
