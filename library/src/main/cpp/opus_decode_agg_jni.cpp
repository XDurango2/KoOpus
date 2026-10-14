// File: library/src/main/cpp/opus_decode_agg_jni.cpp
#include <jni.h>
#include <opus.h>
#include <android/log.h>

// Helper: "nativeHandle" de OpusDecoder (object de Kotlin => campo estático; se prueba también como instancia).
static OpusDecoder* get_dec(JNIEnv* env, jobject thiz) {
    jclass cls = env->GetObjectClass(thiz);
    jlong h = 0;
    if (jfieldID sfid = env->GetStaticFieldID(cls, "nativeHandle", "J")) {
        h = env->GetStaticLongField(cls, sfid);
    } else {
        env->ExceptionClear();
        jfieldID fid = env->GetFieldID(cls, "nativeHandle", "J");
        if (!fid) { env->ExceptionClear(); return nullptr; }
        h = env->GetLongField(thiz, fid);
    }
    return reinterpret_cast<OpusDecoder*>((intptr_t)h);
}

static void throwISE(JNIEnv* env, const char* msg) {
    jclass ex = env->FindClass("java/lang/IllegalStateException");
    if (ex) env->ThrowNew(ex, msg);
}

extern "C" {

JNIEXPORT jint JNICALL
Java_com_steegler_koopus_OpusDecoder_decodeAggregated(JNIEnv* env, jobject thiz,
    jbyteArray payload, jint length, jint maxFrameSize, jshortArray outPcm) {

    auto* dec = get_dec(env, thiz);
    if (!dec) { throwISE(env, "decoder null"); return -1; }

    jbyte* pay = (jbyte*)env->GetPrimitiveArrayCritical(payload, nullptr);
    jshort* pcm = (jshort*)env->GetPrimitiveArrayCritical(outPcm, nullptr);
    if (!pay || !pcm) { if (pay) env->ReleasePrimitiveArrayCritical(payload, pay, 0);
                        if (pcm) env->ReleasePrimitiveArrayCritical(outPcm, pcm, 0);
                        throwISE(env, "OOM"); return -1; }

    // opus_decode entiende paquetes con varios frames (p. ej. 2x20ms del repacketizer) y necesita
    // el paquete completo con TOC; el buffer de salida debe cubrir la duración total del paquete.
    const unsigned char* data = (const unsigned char*)pay;
    (void)maxFrameSize;
    int capacity = (int)env->GetArrayLength(outPcm);
    int samples = opus_decode(dec, data, length, pcm, capacity, 0);

    env->ReleasePrimitiveArrayCritical(outPcm, pcm, 0);
    env->ReleasePrimitiveArrayCritical(payload, pay, 0);
    if (samples < 0) { throwISE(env, opus_strerror(samples)); return -1; }
    return samples;
}

JNIEXPORT void JNICALL
Java_com_steegler_koopus_OpusDecoder_reset(JNIEnv* env, jobject thiz) {
    auto* dec = get_dec(env, thiz);
    if (!dec) { throwISE(env, "decoder null"); return; }
    opus_decoder_ctl(dec, OPUS_RESET_STATE);
}

} // extern "C"
