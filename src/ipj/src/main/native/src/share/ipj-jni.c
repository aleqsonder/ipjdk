#include <assert.h>
#include <stdint.h>
#include <jni.h>

#include "shmem.h"


// <jni_management>

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
	return ipj_shm_init() ? JNI_ERR : JNI_VERSION_1_1;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {
	ipj_shm_cleanup();
}

// </jni_management>


// <SharedMemoryAllocator>

JNIEXPORT jlong JNICALL Java_ipj_SharedMemoryAllocator_shprovide(
	JNIEnv *jenv, jclass jcls, jstring id, jlong size
) {
	assert(jenv);
	assert(id);

	const char *const id_utf = (*jenv)->GetStringUTFChars(jenv, id, NULL);
	size_t size_shuttle = size;

	bool isNew = false;

	const ipj_shm_handle_t
	handle = ipj_shm_provide(id_utf, &size_shuttle, &isNew);

	(*jenv)->ReleaseStringUTFChars(jenv, id, id_utf);

	if (handle == IPJ_SHM_INVALID_HANDLE)
		return IPJ_SHM_INVALID_HANDLE;

	return isNew ? -handle : handle;
}

JNIEXPORT jlong JNICALL Java_ipj_SharedMemoryAllocator_shalloc(
	JNIEnv *jenv, jclass jcls, jlong size
) { return ipj_shm_alloc(size); }

JNIEXPORT void JNICALL Java_ipj_SharedMemoryAllocator_shrelease(
	JNIEnv *jenv, jclass jcls, jlong handle
) { ipj_shm_release(handle); }

// </SharedMemoryAllocator>


// <MemoryBlock>

#define DEF_MEMORY_BLOCK_SET(BIT_COUNT, JTYPE) \
	JNIEXPORT void JNICALL Java_ipj_MemoryBlock_set##BIT_COUNT( \
		JNIEnv *jenv, jclass jcls, \
		jlong handle, jlong offsetBytes, JTYPE value \
	) { ipj_shm_write(handle, offsetBytes, &value, sizeof(value)); }

DEF_MEMORY_BLOCK_SET(8, jbyte)
DEF_MEMORY_BLOCK_SET(16, jshort)
DEF_MEMORY_BLOCK_SET(32, jint)
DEF_MEMORY_BLOCK_SET(64, jlong)

#undef DEF_MEMORY_BLOCK_SET


#define DEF_MEMORY_BLOCK_GET(BIT_COUNT, JTYPE) \
	JNIEXPORT JTYPE JNICALL Java_ipj_MemoryBlock_get##BIT_COUNT( \
		JNIEnv *jenv, jclass jcls, \
		jlong handle, jlong offsetBytes \
	) { \
		JTYPE value; \
		ipj_shm_read(handle, offsetBytes, &value, sizeof(value)); \
		return value; \
	}

DEF_MEMORY_BLOCK_GET(8, jbyte)
DEF_MEMORY_BLOCK_GET(16, jshort)
DEF_MEMORY_BLOCK_GET(32, jint)
DEF_MEMORY_BLOCK_GET(64, jlong)

#undef DEF_MEMORY_BLOCK_GET

// </MemoryBlock>
