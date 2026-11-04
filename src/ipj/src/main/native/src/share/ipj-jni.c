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


// <Shared>

JNIEXPORT jint JNICALL Java_ipj_Shared_getInt(
	JNIEnv *jenv,
	jclass jcls,
	jlong ptr,
	jint offset
) { return *(int *)((char *)ptr + offset); }

JNIEXPORT void JNICALL Java_ipj_Shared_set(
	JNIEnv *jenv,
	jclass jcls,
	jlong ptr,
	jint offset,
	jint value
) { *(int *)((char *)ptr + offset) = value; }

JNIEXPORT jlong JNICALL Java_ipj_Shared_shalloc(
	JNIEnv *jenv,
	jclass jcls,
	jstring id,
	jint size
) {
	assert(jenv);
	assert(id);

	const char *const id_utf = (*jenv)->GetStringUTFChars(jenv, id, NULL);
	size_t size_shuttle = size;

	const ipj_shm_handle_t
	handle = ipj_shm_provide(id_utf, &size_shuttle, NULL);

	(*jenv)->ReleaseStringUTFChars(jenv, id, id_utf);

	return handle;
}

JNIEXPORT void JNICALL Java_ipj_Shared_shfree(
	JNIEnv *jenv,
	jclass jcls,
	jlong handle
) {
	assert(jenv);
	ipj_shm_release(handle);
}

// </Shared>


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
