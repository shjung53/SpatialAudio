//
// Created by shjung on 2024-11-29.
//

#include "SpatialAudioPlayer.h"
#include <oboe/Oboe.h>
#include <vector>
#include <fstream>
#include <cstring> // for memcpy


using namespace oboe;

// Global PCM data buffer (replace with a dynamic loader if needed)
std::vector<float> pcmBuffer;
std::vector<uint8_t> SpatialAudioPlayer::mPcmData;
size_t readIndex = 0; // Track the current playback position

void loadPcmFile(const std::vector<uint8_t> &pcmData) {
    if (pcmData.empty()) {
        throw std::runtime_error("PCM data is empty");
    }

    // 8-bit unsigned PCM 데이터를 float (-1.0 ~ 1.0)로 변환
    for (uint8_t sample: pcmData) {
        float normalizedSample = (sample / 127.5f) - 1.0f; // Normalize to -1.0 ~ 1.0
        pcmBuffer.push_back(normalizedSample);
    }
}

oboe::Result SpatialAudioPlayer::open(oboe::ChannelMask channelMask, std::vector<uint8_t> pcmData) {
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
    int32_t numSamples = numFrames * audioStream->getChannelCount();

    for (int i = 0; i < numSamples; i++) {
        if (readIndex < pcmBuffer.size()) {
            *output++ = pcmBuffer[readIndex++];
        } else {
            *output++ = 0.0f; // Fill with silence if data ends
        }
    }

    return oboe::DataCallbackResult::Continue;
}

void SpatialAudioPlayer::MyErrorCallback::onErrorAfterClose(oboe::AudioStream *oboeStream,
                                                            oboe::Result error) {
    // Try to open and start a new stream after a disconnect.
    if (mParent->open(oboeStream->getChannelMask(), mPcmData) == Result::OK) {
        mParent->start();
    }
}

oboe::DataCallbackResult
SpatialAudioPlayer::MyPcmLoadCallback::onAudioReady(oboe::AudioStream *audioStream, void *audioData,
                                                    int32_t numFrames) {
    loadPcmFile(mPcmData);
    return DataCallbackResult::Continue;
}
