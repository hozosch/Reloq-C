/* Thin Android transport for the same native C core used by NVDA.
 * A persistent Java worker owns every engine. Only stop crosses threads. */
#include <jni.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "reloq_eci.h"

#define ENGINE_LIMIT 14
#define PCM_SAMPLES 1100
typedef struct {
    int dialect;
    ECIHand handle;
    short pcm[PCM_SAMPLES];
} language_engine;
typedef struct {
    JavaVM *vm;
    pthread_mutex_t guard;
    ECIHand active;
    char *lexicon;
    language_engine engines[ENGINE_LIMIT];
    jobject sink;
    jmethodID write, cancelled;
} android_engine;

static int is_cancelled(JNIEnv *env, android_engine *engine)
{
    jboolean result = (*env)->CallBooleanMethod(env, engine->sink, engine->cancelled);
    return (*env)->ExceptionCheck(env) || result;
}

static int ECICALL receive(ECIHand handle, int message, int samples, void *opaque)
{
    android_engine *engine = opaque;
    JNIEnv *env = NULL;
    int attached = 0, result = 2;
    if ((*engine->vm)->GetEnv(engine->vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
#if defined(__ANDROID__)
        if ((*engine->vm)->AttachCurrentThread(engine->vm, &env, NULL) != JNI_OK)
#else
        if ((*engine->vm)->AttachCurrentThread(engine->vm, (void **)&env, NULL) != JNI_OK)
#endif
            return 2;
        attached = 1;
    }
    if (!is_cancelled(env, engine)) {
        if (message != RELOQ_ECI_WAVEFORM_BUFFER) result = 1;
        else if (samples >= 0 && samples <= PCM_SAMPLES) {
            for (size_t i = 0; i < ENGINE_LIMIT; ++i) {
                if (engine->engines[i].handle != handle) continue;
                jbyteArray data = (*env)->NewByteArray(env, samples * 2);
                if (data) {
                    (*env)->SetByteArrayRegion(env, data, 0, samples * 2,
                        (const jbyte *)engine->engines[i].pcm);
                    if (!(*env)->ExceptionCheck(env) &&
                        (*env)->CallBooleanMethod(env, engine->sink, engine->write, data))
                        result = 1;
                    (*env)->DeleteLocalRef(env, data);
                }
                break;
            }
        }
    }
    /* Exceptions on an attached native thread cannot be delivered to the
     * worker. Abort output and let the synchronous call report failure. */
    if (attached) {
        if ((*env)->ExceptionCheck(env)) { (*env)->ExceptionClear(env); result = 2; }
        (*engine->vm)->DetachCurrentThread(engine->vm);
    }
    return result;
}

JNIEXPORT jlong JNICALL
Java_org_reloq_tts_preview_NativeEngine_create(JNIEnv *env, jclass type, jstring lexicon)
{
    (void)type;
    android_engine *engine = calloc(1, sizeof *engine);
    if (!engine) return 0;
    if ((*env)->GetJavaVM(env, &engine->vm) != JNI_OK ||
        pthread_mutex_init(&engine->guard, NULL)) { free(engine); return 0; }
    const char *path = (*env)->GetStringUTFChars(env, lexicon, NULL);
    if (path) {
        size_t n = strlen(path) + 1;
        engine->lexicon = malloc(n);
        if (engine->lexicon) memcpy(engine->lexicon, path, n);
        (*env)->ReleaseStringUTFChars(env, lexicon, path);
    }
    if (!engine->lexicon) {
        pthread_mutex_destroy(&engine->guard); free(engine); return 0;
    }
    return (jlong)(intptr_t)engine;
}

JNIEXPORT jboolean JNICALL
Java_org_reloq_tts_preview_NativeEngine_synthesize(JNIEnv *env, jclass type,
    jlong pointer, jint dialect, jintArray controls, jbyteArray text, jobject sink)
{
    (void)type;
    android_engine *engine = (android_engine *)(intptr_t)pointer;
    if (!engine || (*env)->GetArrayLength(env, controls) != 10) return JNI_FALSE;
    engine->sink = (*env)->NewGlobalRef(env, sink);
    if (!engine->sink) return JNI_FALSE;
    jclass sink_type = (*env)->GetObjectClass(env, sink);
    engine->write = (*env)->GetMethodID(env, sink_type, "write", "([B)Z");
    engine->cancelled = (*env)->GetMethodID(env, sink_type, "isCancelled", "()Z");
    (*env)->DeleteLocalRef(env, sink_type);
    int ok = 0;
    ECIHand handle = NULL;
    if (!engine->write || !engine->cancelled || is_cancelled(env, engine)) goto done;
    language_engine *slot = NULL;
    for (size_t i = 0; i < ENGINE_LIMIT; ++i)
        if (engine->engines[i].handle && engine->engines[i].dialect == dialect)
            { slot = &engine->engines[i]; break; }
    if (!slot) {
        for (size_t i = 0; i < ENGINE_LIMIT; ++i)
            if (!engine->engines[i].handle) { slot = &engine->engines[i]; break; }
        if (!slot) goto done;
        handle = eciNewEx(dialect);
        if (!handle) goto done;
        eciSetParam(handle, RELOQ_ECI_VOICE_MODE, RELOQ_ECI_VOICE_GENERIC);
        if ((dialect == RELOQ_ECI_GERMAN && !eciLoadLexicon(handle, engine->lexicon)) ||
            eciSetOutputBuffer(handle, PCM_SAMPLES, slot->pcm) <= 0) {
            eciDelete(handle); handle = NULL; goto done;
        }
        slot->dialect = dialect;
        slot->handle = handle;
        eciRegisterCallback(handle, receive, engine);
    }
    handle = slot->handle;
    pthread_mutex_lock(&engine->guard);
    engine->active = handle;
    pthread_mutex_unlock(&engine->guard);
    if (is_cancelled(env, engine)) goto done;
    jint values[10];
    static const int parameters[10] = {
        RELOQ_ECI_GENDER, RELOQ_ECI_HEAD_SIZE, RELOQ_ECI_NATIVE_PITCH,
        RELOQ_ECI_NATIVE_INFLECTION, RELOQ_ECI_ROUGHNESS,
        RELOQ_ECI_BREATHINESS, RELOQ_ECI_NATIVE_VOLUME,
        RELOQ_ECI_NATIVE_SPEED, RELOQ_ECI_SAMPLE_RATE, RELOQ_ECI_PAUSE_MODE
    };
    (*env)->GetIntArrayRegion(env, controls, 0, 10, values);
    if ((*env)->ExceptionCheck(env)) goto done;
    for (size_t i = 0; i < 10; ++i)
        if (eciSetParam(handle, parameters[i], values[i]) < 0) goto done;
    eciClearInput(handle);
    jsize length = (*env)->GetArrayLength(env, text);
    jbyte *bytes = (*env)->GetByteArrayElements(env, text, NULL);
    if (!bytes) goto done;
    int accepted = eciAddTextUtf8(handle, bytes, (size_t)length, 0);
    (*env)->ReleaseByteArrayElements(env, text, bytes, JNI_ABORT);
    if (accepted && !is_cancelled(env, engine))
        ok = eciSynthesize(handle) && eciSynchronize(handle);
done:
    pthread_mutex_lock(&engine->guard);
    engine->active = NULL;
    pthread_mutex_unlock(&engine->guard);
    if (handle) eciClearInput(handle);
    (*env)->DeleteGlobalRef(env, engine->sink);
    engine->sink = NULL;
    return ok ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_org_reloq_tts_preview_NativeEngine_stop(JNIEnv *env, jclass type, jlong pointer, jobject sink)
{
    (void)type;
    android_engine *engine = (android_engine *)(intptr_t)pointer;
    if (!engine) return;
    pthread_mutex_lock(&engine->guard);
    if (engine->active && (*env)->IsSameObject(env, engine->sink, sink))
        eciStop(engine->active);
    pthread_mutex_unlock(&engine->guard);
}

JNIEXPORT void JNICALL
Java_org_reloq_tts_preview_NativeEngine_destroy(JNIEnv *env, jclass type, jlong pointer)
{
    (void)env; (void)type;
    android_engine *engine = (android_engine *)(intptr_t)pointer;
    if (!engine) return;
    for (size_t i = 0; i < ENGINE_LIMIT; ++i)
        if (engine->engines[i].handle) eciDelete(engine->engines[i].handle);
    pthread_mutex_destroy(&engine->guard);
    free(engine->lexicon); free(engine);
}
