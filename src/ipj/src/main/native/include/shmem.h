#ifndef IPJ_SHMEM_H
#define IPJ_SHMEM_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

int ipj_shm_init(void);
void ipj_shm_cleanup(void);

typedef uint32_t ipj_shm_handle_t;
#define IPJ_SHM_HANDLE_INVALID 0

ipj_shm_handle_t ipj_shm_alloc(size_t size);

ipj_shm_handle_t ipj_shm_provide(
	// [in]
	const char *name,

	// [inout, nullable]
	size_t *size,

	// [out, nullable]
	bool *is_new);

void ipj_shm_release(ipj_shm_handle_t);

// returns -1 on the invalid handle
// returns the unread byte count otherwise
ssize_t ipj_shm_read(
	ipj_shm_handle_t handle,
	size_t offset,
	void *dest,
	size_t size);

// returns -1 on the invalid handle
// returns the unwrite byte count otherwise
ssize_t ipj_shm_write(
	ipj_shm_handle_t handle,
	size_t offset,
	const void *src,
	size_t size);

#endif
