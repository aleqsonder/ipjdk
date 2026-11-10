#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "shmmap.h"

struct ipj_shm { size_t size; void *data; };

ipj_shm_t *ipj_shm_open(const char *name, size_t size, bool *is_new) {
	assert(name);
	
	ipj_shm_t *shm = NULL;

	int fd = shm_open(name, O_RDWR|O_CREAT|O_EXCL, 0666);
	const bool local_is_new = (fd != -1);
	if (fd == -1)
		fd = shm_open(name, O_RDWR, 0666);

	if (ftruncate(fd, size))
		goto fd_cleanup;

	void *const data = mmap(NULL, size, PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED)
		goto fd_cleanup;

	shm = malloc(sizeof(ipj_shm_t));
	if (!shm)
		goto map_cleanup;

	*shm = (ipj_shm_t) { .size = size, .data = data };

	if (is_new)
		*is_new = local_is_new;

	goto fd_cleanup;

map_cleanup:
	munmap(data, size);

fd_cleanup:
	shm_unlink(name);

	return shm;
}

void ipj_shm_close(ipj_shm_t *shm) {
	if (shm) {
		munmap(shm->data, shm->size);
		free(shm);
	}
}

void *ipj_shm_data(const ipj_shm_t *shm) {
	return shm->data;
}
