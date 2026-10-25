#include <windows.h>
#include "shmmap.h"


void *shmopen(const char *name, size_t size) {
	const DWORD
		size_hi = size >> (8 * sizeof(DWORD)),
		size_lo = size & 0xffffffff;

	const HANDLE fmap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		size_hi,
		size_lo,
		name);
	if (fmap == NULL)
		return NULL;

	const LPVOID mem = MapViewOfFile(fmap, FILE_MAP_WRITE, 0, 0, 0);

	CloseHandle(fmap);
	return mem;
}

void shmclose(void *shm) {
	if (shm)
		UnmapViewOfFile(shm);
}
