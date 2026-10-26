#include <assert.h>
#include <jni.h>

#include "shmem.h"


jint JNI_OnLoad(JavaVM *vm, void *reserved) {
	return ipj_shm_init() ? JNI_ERR : JNI_VERSION_1_1;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {
	ipj_shm_cleanup();
}


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
	void *const shm = ipj_shalloc(id_utf, &size_shuttle, NULL);
	(*jenv)->ReleaseStringUTFChars(jenv, id, id_utf);

	return (jlong)shm;
}

JNIEXPORT void JNICALL Java_ipj_Shared_shfree(
	JNIEnv *jenv,
	jclass jcls,
	jlong ptr
) {
	assert(jenv);
	ipj_shfree((void *)ptr);
}
