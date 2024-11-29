//
// Created by shjung on 2024-11-29.
//

#include "SpatialAudioPlayer.h"
#include <oboe/Oboe.h>

using namespace oboe;

oboe::Result SpatialAudioPlayer::open(oboe::ChannelMask channelMask) {
    mDataCallback = std::make_shared<MyDataCallback>();
    mErrorCallback = std::make_shared<MyErrorCallback>(this);

    AudioStreamBuilder builder;
    oboe::Result result = builder.setSharingMode(oboe::SharingMode::Exclusive)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setFormat(oboe::AudioFormat::Float)
            ->setChannelCount(kChannelCount)
            ->setChannelMask(channelMask)
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
    // We requested float when we built the stream.
    float *output = (float *) audioData;
    // Fill buffer with random numbers to create "white noise".
    int numSamples = numFrames;
    for (int i = 0; i < numSamples; i++) {
        // drand48() returns a random number between 0.0 and 1.0.
        // Center and scale it to a reasonable value.
        *output++ = (float) ((drand48() - 0.5) * 0.6);
    }
    return oboe::DataCallbackResult::Continue;
}

void SpatialAudioPlayer::MyErrorCallback::onErrorAfterClose(oboe::AudioStream *oboeStream,
                                                            oboe::Result error) {
    // Try to open and start a new stream after a disconnect.
    if (mParent->open(oboeStream->getChannelMask()) == Result::OK) {
        mParent->start();
    }
}
