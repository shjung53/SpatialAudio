/*
 * Copyright 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

// JNI functions are "C" calling convention
#ifdef __cplusplus
extern "C" {
#endif

using namespace oboe;

// Use a static object so we don't have to worry about it getting deleted at the wrong time.
static SpatialAudioPlayer sPlayer;

/**
 * Native (JNI) implementation of AudioPlayer.startAudiostreamNative()
 */
JNIEXPORT jint JNICALL Java_com_shjung53_audioplayer_AudioPlayer_startAudioStream(
        JNIEnv *env, jobject, jbyteArray byteArray) {
    // 1. jbyteArray를 네이티브 데이터로 변환
    jsize length = env->GetArrayLength(byteArray); // byteArray의 길이 가져오기
    jbyte *data = env->GetByteArrayElements(byteArray, nullptr); // 데이터 포인터 가져오기
    __android_log_print(ANDROID_LOG_DEBUG, "바이트어레이", "ByteArraySize: %d", length);

    std::vector<float> floatBuffer(length / 2); // 16-bit이므로 요소 개수는 절반
    const int16_t* pcmData = reinterpret_cast<const int16_t*>(data);

    for (jsize i = 0; i < length / 2; ++i) {
        floatBuffer[i] = static_cast<float>(pcmData[i]) / 32768.0f; // Normalize to -1.0 ~ 1.0
    }

    // 3. jbyteArray 메모리 해제
    env->ReleaseByteArrayElements(byteArray, data, 0); // JNI 자원 반환

    Result result = sPlayer.open(ChannelMask::CM7Point1, floatBuffer);
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
