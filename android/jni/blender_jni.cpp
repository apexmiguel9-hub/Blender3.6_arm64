/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Blender 2.79b Android JNI Wrapper
 * Bridges Blender's C API to Android Java via JNI
 */

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations from Blender */
extern "C" {
int main_python_enter(int argc, const char **argv);
void main_python_exit(void);

/* GHOST Android C API (from GHOST_SystemAndroid.cpp) */
int GHOST_Android_init(JNIEnv *env, jobject surface, int w, int h);
int GHOST_Android_setSurface(JNIEnv *env, jobject surface, int w, int h);
int GHOST_Android_touchEvent(int action, float x, float y);
int GHOST_Android_keyEvent(int down, int keyCode, int metaState);
}

static JavaVM *g_jvm = NULL;
static bool g_blender_initialized = false;

extern "C" {

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
	g_jvm = vm;
	return JNI_VERSION_1_6;
}

JNIEXPORT jint JNICALL
Java_com_blender_android_BlenderLib_initBlender(
        JNIEnv *env, jclass clazz,
        jstring data_path, jstring temp_path)
{
	if (g_blender_initialized)
		return 0;

	const char *data = env->GetStringUTFChars(data_path, NULL);
	const char *temp = env->GetStringUTFChars(temp_path, NULL);

	setenv("BLENDER_SYSTEM_SCRIPTS", data, 1);
	setenv("BLENDER_USER_SCRIPTS", data, 1);
	setenv("BLENDER_SYSTEM_DATAFILES", data, 1);
	setenv("TMP", temp, 1);
	setenv("TMPDIR", temp, 1);

	const char *argv[] = {
		"blender",
		"--background",
		NULL
	};

	int result = main_python_enter(1, argv);
	g_blender_initialized = (result == 0);

	env->ReleaseStringUTFChars(data_path, data);
	env->ReleaseStringUTFChars(temp_path, temp);

	return result;
}

JNIEXPORT jint JNICALL
Java_com_blender_android_BlenderLib_initSurface(
        JNIEnv *env, jclass clazz,
        jobject surface, jint width, jint height)
{
	return GHOST_Android_init(env, surface, width, height);
}

JNIEXPORT jint JNICALL
Java_com_blender_android_BlenderLib_touchEvent(
        JNIEnv *env, jclass clazz,
        jint action, jfloat x, jfloat y)
{
	return GHOST_Android_touchEvent(action, x, y);
}

JNIEXPORT jint JNICALL
Java_com_blender_android_BlenderLib_keyEvent(
        JNIEnv *env, jclass clazz,
        jint down, jint keyCode, jint metaState)
{
	return GHOST_Android_keyEvent(down, keyCode, metaState);
}

JNIEXPORT void JNICALL
Java_com_blender_android_BlenderLib_exitBlender(
        JNIEnv *env, jclass clazz)
{
	if (g_blender_initialized) {
		main_python_exit();
		g_blender_initialized = false;
	}
}

} /* extern "C" */
