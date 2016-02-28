#include <stdint.h>
#include <jni.h>
#include "common.h"
#include "debug.h"
#include <x264.h>

static jclass H264Parameters_class = NULL;
static jfieldID H264Parameters_object_fid = NULL;

static x264_param_t* getPointer(JNIEnv* env, jobject self) {
	if (H264Parameters_class == NULL) {
		dprint(LOG_V4L4J, "[PTR] Obtaining class\n");
		H264Parameters_class = (*env)->GetObjectClass(env, self);
		dprint(LOG_V4L4J, "[PTR] Got class %lu\n", (long unsigned int)H264Parameters_class);
		H264Parameters_object_fid = (*env)->GetFieldID(env, H264Parameters_class, "object", "J");
		dprint(LOG_V4L4J, "[PTR] Got field id for 'object': %lu\n", (long unsigned int)H264Parameters_object_fid);
	}
	
	long ptr = (*env)->GetLongField(env, self, H264Parameters_object_fid);
	dprint(LOG_V4L4J, "[PTR] Got pointer address to x264_param_t: %#08x\n", ptr);
	return (struct x264_param_t*) (uintptr_t) ptr;
}

JNIEXPORT jlong JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_allocate(JNIEnv * env, jclass clazz) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	struct x264_param_t* params;
	XMALLOC(params, x264_param_t*, sizeof(x264_param_t));
	
	return (uintptr_t) params;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_close(JNIEnv* env, jobject self) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	XFREE(params);
	return;
}
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_initDefault(JNIEnv* env, jobject self) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	x264_param_default(params);
}
static const char * const x264_preset_names[] = { "ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow", "placebo", 0 };
static const char * const x264_tune_names[] = { "film", "animation", "grain", "stillimage", "psnr", "ssim", "fastdecode", "zerolatency", 0 };
static const char * const x264_profile_names[] = { "baseline", "main", "high", "high10", "high422", "high444", 0 };
/*
 * Class:     au_edu_jcu_v4l4j_encoder_h264_H264Parameters
 * Method:    initWithDefaultPreset
 * Signature: (JLjava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_initWithPreset(JNIEnv* env, jobject self, jint preset, jint tune) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	if (preset > 10)
		return -1;
	
	if (tune > 8)
		return -1;
	char* preset_name = x264_preset_names[preset];
	char* tune_name = x264_tune_names[tune];
	
	const jchar* c_preset = (*env)->GetStringChars(env, preset, NULL);
	const jchar* c_tune = (*env)->GetStringChars(env, tune, NULL);
	
	dprint(LOG_V4L4J, "[PARAM] Initializing with preset '%s', tune '%s'\n", preset_name, tune_name);
	int result = x264_param_default_preset(params, preset_name, tune_name);
	
	return result;
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_h264_H264Parameters
 * Method:    applyFastFirstPass
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_applyFastFirstPass(JNIEnv* env, jobject self) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	x264_param_apply_fastfirstpass(params);
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_applyProfile(JNIEnv* env, jobject self, jint profile) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	if (profile > 6 || profile < 0)
		return;
	
	const char* profile_name = x264_profile_names[profile];
	
	dprint(LOG_V4L4J, "[PARAM] Applying profile '%s'\n", profile_name);
	x264_param_apply_profile(params, profile_name);
}

static inline jint setParamByName(x264_param_t* params, JNIEnv* env, jstring key, const char* value) {
	const jchar* c_key = (*env)->GetStringChars(env, key, NULL);
	
	int result = x264_param_parse(params, (char*) c_key, value);
	
	(*env)->ReleaseStringChars(env, key, c_key);
	
	return result;
}
/*
 * Class:     au_edu_jcu_v4l4j_encoder_h264_H264Parameters
 * Method:    setParamByName
 * Signature: (JLjava/lang/String;Z)I
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setParamByName__JLjava_lang_String_2Z(JNIEnv* env, jobject self, jstring name, jboolean value) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	return setParamByName(params, env, name, (const char*) &value);
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_h264_H264Parameters
 * Method:    setParamByName
 * Signature: (JLjava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setParamByName__JLjava_lang_String_2I(JNIEnv* env, jobject self, jstring name, jint value) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	return setParamByName(params, env, name, (const char*) &value);
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_h264_H264Parameters
 * Method:    setParamByName
 * Signature: (JLjava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setParamByName__JLjava_lang_String_2Ljava_lang_String_2(JNIEnv* env, jobject self, jstring name, jstring value) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	const jchar* c_value = (*env)->GetStringChars(env, value, NULL);
	int result = setParamByName(params, env, name, (const char*) &value);
	(*env)->ReleaseStringChars(env, value, c_value);
	
	return result;
}

/*
 * Class:     au_edu_jcu_v4l4j_encoder_h264_H264Parameters
 * Method:    setInputDimension
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setInputDimension(JNIEnv* env, jobject self, jint width, jint height) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	params->i_width = width;
	params->i_height = height;
}
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setCsp(JNIEnv* env, jobject self, jint csp) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	params->i_csp = csp;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setVfrInput(JNIEnv* env, jobject self, jboolean value) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	params->b_vfr_input = value;
}

JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setRepeatHeaders(JNIEnv* env, jobject self, jboolean value) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	params->b_repeat_headers = value;
}
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setAnnexb(JNIEnv* env, jobject self, jboolean value) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	params->b_annexb = value;
}

JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_getCsp(JNIEnv* env, jobject self) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	return params->i_csp;
}
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_getWidth(JNIEnv* env, jobject self) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	return params->i_width;
}
JNIEXPORT jint JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_getHeight(JNIEnv* env, jobject self) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	return params->i_height;
}
/*
 * Class:     au_edu_jcu_v4l4j_encoder_h264_H264Parameters
 * Method:    setCrop
 * Signature: (JIIII)V
 */
JNIEXPORT void JNICALL Java_au_edu_jcu_v4l4j_encoder_h264_H264Parameters_setCrop(JNIEnv* env, jobject self, jint left, jint top, jint right, jint bottom) {
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	x264_param_t* params = getPointer(env, self);
	
	params->crop_rect.i_left = left;
	params->crop_rect.i_top = top;
	params->crop_rect.i_right = right;
	params->crop_rect.i_bottom = bottom;
}