#include <stdint.h>
#include <windows.h>

#include "shmmap.h"

struct ipj_shm { HANDLE file_mapping; void *data; };

ipj_shm_t *ipj_shm_open(const char *name, size_t size, bool *is_new) {
	const DWORD
		size_hi = (uint64_t)size >> (8 * sizeof(DWORD)),
		size_lo = (uint64_t)size & 0xffffffff;

	ipj_shm_t *shm = NULL;

	const HANDLE fmap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		size_hi,
		size_lo,
		name);
	const DWORD last_err = GetLastError();
	
	if (fmap == NULL) {
		SetLastError(last_err);
		goto final;
	}

	if (is_new)
		*is_new = (last_err != ERROR_ALREADY_EXISTS);

	const LPVOID view = MapViewOfFile(fmap, FILE_MAP_WRITE, 0, 0, size);
	if (view == NULL)
		goto fmap_cleanup;

	shm = malloc(sizeof(*shm));
	if (shm == NULL)
		goto view_cleanup;

	*shm = (ipj_shm_t) { .file_mapping = fmap, .data = view };

	goto final;

view_cleanup:
	UnmapViewOfFile(view);

fmap_cleanup:
	CloseHandle(fmap);

final:
	return shm;
}

void ipj_shm_close(ipj_shm_t *shm) {
	if (shm) {
		UnmapViewOfFile(shm->data);
		CloseHandle(shm->file_mapping);
		free(shm);
	}
}

void *ipj_shm_data(const ipj_shm_t *shm) {
	return shm ? shm->data : NULL;
}
