// File: library/src/main/cpp/opus_tuning_jni.cpp
// New JNI file: runtime CTLs for Opus encoder (keeps opus_wrapper.cpp untouched).

#include <jni.h>
#include <opus.h>
#include <android/log.h>

// Helper: OpusTuning.encoder -> OpusEncoder (Kotlin object) -> long "nativeHandle".
// En un `object` de Kotlin, las propiedades son campos estáticos; se prueba también como instancia.
static OpusEncoder* get_enc(JNIEnv* env, jobject thiz) {
    jclass tcls = env->GetObjectClass(thiz);
    jfieldID efid = env->GetFieldID(tcls, "encoder", "Ljava/lang/Object;");
    if (!efid) { env->ExceptionClear(); return nullptr; }
    jobject encoder = env->GetObjectField(thiz, efid);
    if (!encoder) return nullptr;
    jclass ecls = env->GetObjectClass(encoder);
    jlong h = 0;
    if (jfieldID sfid = env->GetStaticFieldID(ecls, "nativeHandle", "J")) {
        h = env->GetStaticLongField(ecls, sfid);
    } else {
        env->ExceptionClear();
        jfieldID ifid = env->GetFieldID(ecls, "nativeHandle", "J");
        if (!ifid) { env->ExceptionClear(); return nullptr; }
        h = env->GetLongField(encoder, ifid);
    }
    return reinterpret_cast<OpusEncoder*>((intptr_t)h);
}

static void throwISE(JNIEnv* env, const char* msg) {
    jclass ex = env->FindClass("java/lang/IllegalStateException");
    if (ex) env->ThrowNew(ex, msg);
}

extern "C" {

JNIEXPORT void JNICALL
Java_com_steegler_koopus_OpusTuning_setBitrate(JNIEnv* env, jobject thiz, jint bps) {
    auto* enc = get_enc(env, thiz); if (!enc) { throwISE(env, "encoder null"); return; }
    opus_int32 v = (opus_int32)bps;
    int rc = opus_encoder_ctl(enc, OPUS_SET_BITRATE(v));
    if (rc < 0) throwISE(env, "OPUS_SET_BITRATE failed");
}

JNIEXPORT void JNICALL
Java_com_steegler_koopus_OpusTuning_setComplexity(JNIEnv* env, jobject thiz, jint level) {
    auto* enc = get_enc(env, thiz); if (!enc) { throwISE(env, "encoder null"); return; }
    int clamped = level < 0 ? 0 : (level > 10 ? 10 : level);
    int rc = opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(clamped));
    if (rc < 0) throwISE(env, "OPUS_SET_COMPLEXITY failed");
}

JNIEXPORT void JNICALL
Java_com_steegler_koopus_OpusTuning_setMaxBandwidth(JNIEnv* env, jobject thiz, jint hz) {
    auto* enc = get_enc(env, thiz); if (!enc) { throwISE(env, "encoder null"); return; }
    int bw = OPUS_BANDWIDTH_WIDEBAND;
    switch (hz) {
        case 8000:  bw = OPUS_BANDWIDTH_NARROWBAND;    break;
        case 16000: bw = OPUS_BANDWIDTH_WIDEBAND;      break;
        case 24000: bw = OPUS_BANDWIDTH_SUPERWIDEBAND; break;
        case 48000: bw = OPUS_BANDWIDTH_FULLBAND;      break;
        default:    bw = OPUS_BANDWIDTH_WIDEBAND;      break;
    }
    int rc = opus_encoder_ctl(enc, OPUS_SET_MAX_BANDWIDTH(bw));
    if (rc < 0) throwISE(env, "OPUS_SET_MAX_BANDWIDTH failed");
}

JNIEXPORT void JNICALL
Java_com_steegler_koopus_OpusTuning_setDTX(JNIEnv* env, jobject thiz, jboolean on) {
    auto* enc = get_enc(env, thiz); if (!enc) { throwISE(env, "encoder null"); return; }
    int rc = opus_encoder_ctl(enc, OPUS_SET_DTX(on ? 1 : 0));
    if (rc < 0) throwISE(env, "OPUS_SET_DTX failed");
}

JNIEXPORT void JNICALL
Java_com_steegler_koopus_OpusTuning_setInbandFEC(JNIEnv* env, jobject thiz, jboolean on) {
    auto* enc = get_enc(env, thiz); if (!enc) { throwISE(env, "encoder null"); return; }
    int rc = opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(on ? 1 : 0));
    if (rc < 0) throwISE(env, "OPUS_SET_INBAND_FEC failed");
}

JNIEXPORT void JNICALL
Java_com_steegler_koopus_OpusTuning_setCBR(JNIEnv* env, jobject thiz, jboolean cbr) {
    auto* enc = get_enc(env, thiz); if (!enc) { throwISE(env, "encoder null"); return; }
    int rc = opus_encoder_ctl(enc, OPUS_SET_VBR(cbr ? 0 : 1));
    if (rc < 0) throwISE(env, "OPUS_SET_VBR failed");
}

JNIEXPORT void JNICALL
Java_com_steegler_koopus_OpusTuning_setVBRConstraint(JNIEnv* env, jobject thiz, jboolean on) {
    auto* enc = get_enc(env, thiz); if (!enc) { throwISE(env, "encoder null"); return; }
    int rc = opus_encoder_ctl(enc, OPUS_SET_VBR_CONSTRAINT(on ? 1 : 0));
    if (rc < 0) throwISE(env, "OPUS_SET_VBR_CONSTRAINT failed");
}

JNIEXPORT void JNICALL
Java_com_steegler_koopus_OpusTuning_setSignalVoice(JNIEnv* env, jobject thiz) {
    auto* enc = get_enc(env, thiz); if (!enc) { throwISE(env, "encoder null"); return; }
    int rc = opus_encoder_ctl(enc, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
    if (rc < 0) throwISE(env, "OPUS_SET_SIGNAL failed");
}

JNIEXPORT void JNICALL
Java_com_steegler_koopus_OpusTuning_setPacketLossPercent(JNIEnv* env, jobject thiz, jint pct) {
    auto* enc = get_enc(env, thiz); if (!enc) { throwISE(env, "encoder null"); return; }
    int p = pct < 0 ? 0 : (pct > 100 ? 100 : pct);
    int rc = opus_encoder_ctl(enc, OPUS_SET_PACKET_LOSS_PERC(p));
    if (rc < 0) throwISE(env, "OPUS_SET_PACKET_LOSS_PERC failed");
}

} // extern "C"
