
#include <stdlib.h>
#include "ifmuslib.h"
#include "net_autch_android_pceplayer_PMDPlayerThread.h"

JNIEXPORT void JNICALL Java_net_autch_android_pceplayer_PMDPlayerThread_muslib_1init
  (JNIEnv *env, jclass klass)
{
	muslib_init();
}

JNIEXPORT jint JNICALL Java_net_autch_android_pceplayer_PMDPlayerThread_muslib_1load_1from_1file
  (JNIEnv *env, jclass klass, jstring jfilename)
{
	jint ret = 0;

	const char* filename = (*env)->GetStringUTFChars(env, jfilename, NULL);

	ret = muslib_load_from_file(filename);

	(*env)->ReleaseStringUTFChars(env, jfilename, filename);

	return ret;
}

JNIEXPORT void JNICALL Java_net_autch_android_pceplayer_PMDPlayerThread_muslib_1start
  (JNIEnv *env, jclass klass)
{
	muslib_start();
}

JNIEXPORT jint JNICALL Java_net_autch_android_pceplayer_PMDPlayerThread_muslib_1render
  (JNIEnv *env, jclass klass, jbyteArray jbuffer, jint size)
{
	jbyte* buffer = (*env)->GetByteArrayElements(env, jbuffer, NULL);

	muslib_render(buffer, size);

	(*env)->ReleaseByteArrayElements(env, jbuffer, buffer, 0);
}

JNIEXPORT void JNICALL Java_net_autch_android_pceplayer_PMDPlayerThread_muslib_1close
  (JNIEnv *env, jclass klass)
{
	muslib_close();
}
