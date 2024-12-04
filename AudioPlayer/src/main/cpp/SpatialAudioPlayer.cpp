//
// Created by shjung on 2024-11-29.
//

#include "SpatialAudioPlayer.h"
#include <oboe/Oboe.h>
#include <utility>
#include <vector>
#include <fstream>
#include <cstring> // for memcpy


using namespace oboe;

// Global PCM data buffer (replace with a dynamic loader if needed)
std::vector<float> SpatialAudioPlayer::mPcmBuffer;
size_t readIndex = 0; // Track the current playback position

void loadPcmFile(const std::vector<float> &pcmData) {
    if (pcmData.empty()) {
        throw std::runtime_error("PCM data is empty");
    }
    for (float sample: pcmData) {
        SpatialAudioPlayer::mPcmBuffer.push_back(sample);
    }
}

oboe::Result SpatialAudioPlayer::open(oboe::ChannelMask channelMask, std::vector<float> pcmData) {
    mDataCallback = std::make_shared<MyDataCallback>();
    mErrorCallback = std::make_shared<MyErrorCallback>(this);
    mPcmLoadCallback = std::make_shared<MyPcmLoadCallback>();
    loadPcmFile(pcmData);

    AudioStreamBuilder builder;
    oboe::Result result = builder.setSharingMode(oboe::SharingMode::Exclusive)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setFormat(oboe::AudioFormat::Float)
            ->setChannelCount(kChannelCount)
            ->setChannelMask(channelMask)
            ->setDataCallback(mPcmLoadCallback)
            ->setDataCallback(mDataCallback)
            ->setErrorCallback(mErrorCallback)
                    // Open using a shared_ptr.
            ->openStream(mStream);
    return result;
}

oboe::Result SpatialAudioPlayer::start() {
    return mStream->requestStart();
}


oboe::Result SpatialAudioPlayer::stop() {
    return mStream->requestStop();
}

oboe::Result SpatialAudioPlayer::close() {
    return mStream->close();
}

oboe::DataCallbackResult
SpatialAudioPlayer::MyDataCallback::onAudioReady(
        oboe::AudioStream *audioStream,
        void *audioData,
        int32_t numFrames) {
    float *output = static_cast<float *>(audioData);
    int32_t channelCount = audioStream->getChannelCount(); // 7채널

    for (int i = 0; i < numFrames; ++i) {
        if (readIndex < mPcmBuffer.size()) {
            float sample = mPcmBuffer[readIndex++]; // Mono 데이터 샘플 읽기

            for (int ch = 0; ch < channelCount; ++ch) {
                *output++ = sample;
            }
        } else {
            for (int ch = 0; ch < channelCount; ++ch) {
                *output++ = 0.0f; // 데이터가 부족하면 모든 채널 Silence
            }
        }
    }

    return oboe::DataCallbackResult::Continue;
}

void SpatialAudioPlayer::MyErrorCallback::onErrorAfterClose(oboe::AudioStream *oboeStream,
                                                            oboe::Result error) {
    // Try to open and start a new stream after a disconnect.
    if (mParent->open(oboeStream->getChannelMask(), mPcmBuffer) == Result::OK) {
        mParent->start();
    }
}

oboe::DataCallbackResult
SpatialAudioPlayer::MyPcmLoadCallback::onAudioReady(oboe::AudioStream *audioStream, void *audioData,
                                                    int32_t numFrames) {
    loadPcmFile(mPcmBuffer);
    return DataCallbackResult::Continue;
}
