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


// <SharedPointer>

JNIEXPORT jlong JNICALL Java_ipj_memory_SharedPointer_provide(
	JNIEnv *jenv, jclass jcls, jstring id, jlong size
) {
	assert(jenv);
	assert(id);

	size_t size_shuttle = size;

	bool isNew = false;

	const char *const id_utf = (*jenv)->GetStringUTFChars(jenv, id, NULL);
	const ssize_t handle = ipj_shm_provide(id_utf, &size_shuttle, &isNew);
	(*jenv)->ReleaseStringUTFChars(jenv, id, id_utf);

	if (handle == -1)
		return 0;

	return isNew ? -handle : handle;
}

JNIEXPORT jlong JNICALL Java_ipj_memory_SharedPointer_alloc(
	JNIEnv *jenv, jclass jcls, jlong size
) { return ipj_shm_alloc(size); }

JNIEXPORT void JNICALL Java_ipj_memory_SharedPointer_release(
	JNIEnv *jenv, jclass jcls, jlong handle
) { ipj_shm_release(handle); }


#define DEF_MEMORY_BLOCK_SET(BIT_COUNT, JTYPE) \
	JNIEXPORT void JNICALL Java_ipj_memory_SharedPointer_set##BIT_COUNT( \
		JNIEnv *jenv, jclass jcls, \
		jlong handle, jlong offsetBytes, JTYPE value \
	) { \
		void *const ptr = ipj_shm_handle_resolve(handle); \
		if (ptr != NULL) /* FIXME: Throw an exception on NULL */ \
			*(JTYPE *)((intptr_t)ptr + offsetBytes) = value; \
	}

DEF_MEMORY_BLOCK_SET(8, jbyte)
DEF_MEMORY_BLOCK_SET(16, jshort)
DEF_MEMORY_BLOCK_SET(32, jint)
DEF_MEMORY_BLOCK_SET(64, jlong)

#undef DEF_MEMORY_BLOCK_SET


#define DEF_MEMORY_BLOCK_GET(BIT_COUNT, JTYPE) \
	JNIEXPORT JTYPE JNICALL Java_ipj_memory_SharedPointer_get##BIT_COUNT( \
		JNIEnv *jenv, jclass jcls, \
		jlong handle, jlong offsetBytes \
	) { \
		void *const ptr = ipj_shm_handle_resolve(handle); \
		if (ptr != NULL) /* FIXME: Throw an exception on NULL */ \
			return *(JTYPE *)((intptr_t)ptr + offsetBytes); \
		return 0; \
	}

DEF_MEMORY_BLOCK_GET(8, jbyte)
DEF_MEMORY_BLOCK_GET(16, jshort)
DEF_MEMORY_BLOCK_GET(32, jint)
DEF_MEMORY_BLOCK_GET(64, jlong)

#undef DEF_MEMORY_BLOCK_GET

// </SharedPointer>
