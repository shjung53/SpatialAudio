#include <jni.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <android/log.h>
#include <SpatialAudioPlayer.h>
#include <oboe/Oboe.h>
#include <vector>

#include <android/log.h>

#ifdef __cplusplus
extern "C" {
#endif

using namespace oboe;

static SpatialAudioPlayer sPlayer;

/**
 * Native (JNI) implementation of AudioPlayer.startAudiostreamNative()
 */
JNIEXPORT jint JNICALL Java_com_shjung53_audioplayer_AudioPlayer_startAudioStream(
        JNIEnv *env, jobject, jbyteArray byteArray, jint pcmSize) {
    // 1. jbyteArray를 네이티브 데이터로 변환
    jsize length = env->GetArrayLength(byteArray); // byteArray의 길이 가져오기
    jbyte *data = env->GetByteArrayElements(byteArray, nullptr); // 데이터 포인터 가져오기
    __android_log_print(ANDROID_LOG_DEBUG, "바이트어레이", "ByteArraySize: %d", length);

    auto* pcmData = reinterpret_cast<int16_t*>(data);
    // 3. jbyteArray 메모리 해제
    env->ReleaseByteArrayElements(byteArray, data, 0); // JNI 자원 반환

    Result result = sPlayer.open(ChannelMask::Mono, pcmData, pcmSize);
    if (result == Result::OK) {
        result = sPlayer.start();
    }
    return (jint) result;
}

/**
 * Native (JNI) implementation of AudioPlayer.stopAudioStreamNative()
 */
JNIEXPORT jint JNICALL Java_com_shjung53_audioplayer_AudioPlayer_stopAudioStream(
        JNIEnv * /* env */, jobject) {
    // We need to close() even if the stop() fails because we need to delete the resources.
    Result result1 = sPlayer.stop();
    Result result2 = sPlayer.close();
    // Return first failure code.
    return (jint) ((result1 != Result::OK) ? result1 : result2);
}
#ifdef __cplusplus
}
#endif
