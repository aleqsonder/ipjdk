#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

// TEST
#include <stdio.h>

#include "shmmap.h"

struct ipj_shm { size_t size; char *name; void *data; };

ipj_shm_t *ipj_shm_open(const char *name, size_t size, bool *is_new) {
	assert(name);
	
	ipj_shm_t *shm = NULL;

	int fd = shm_open(name, O_RDWR|O_CREAT|O_EXCL, 0666);
	const bool local_is_new = (fd != -1);
	if (fd == -1)
		fd = shm_open(name, O_RDWR, 0666);
	else if (ftruncate(fd, size))
		goto fd_cleanup;

	fputs("shm_open", stderr);

	if (fd == -1)
		goto finish;

	void *const data = mmap(NULL, size, PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED)
		goto fd_cleanup;

	fputs("mmap", stderr);

	char *const name_copy = strdup(name);
	if (!name_copy)
		goto map_cleanup;

	fputs("strdup", stderr);

	shm = malloc(sizeof(ipj_shm_t));
	if (!shm)
		goto name_cleanup;

	fputs("malloc", stderr);

	*shm = (ipj_shm_t) {
		.size = size,
		.name = name_copy,
		.data = data };

	if (is_new)
		*is_new = local_is_new;

	goto finish;

name_cleanup:
	free(name_copy);

map_cleanup:
	munmap(data, size);

fd_cleanup:
	shm_unlink(name);

finish:
	return shm;
}

void ipj_shm_close(ipj_shm_t *shm) {
	if (shm) {
		munmap(shm->data, shm->size);
		shm_unlink(shm->name);
		free(shm->name);
		free(shm);
	}
}

void *ipj_shm_data(const ipj_shm_t *shm) {
	return shm->data;
}
