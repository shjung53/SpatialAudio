//
// Created by shjung on 2024-11-29.
//

#include "SpatialAudioPlayer.h"
#include "../../../../oboe-main/src/common/OboeDebug.h"
#include <oboe/Oboe.h>
#include <utility>
#include <vector>
#include <fstream>
#include <cstring> // for memcpy


using namespace oboe;

std::unique_ptr<oboe::FifoBuffer> SpatialAudioPlayer::buffer = nullptr;
constexpr size_t framesToWrite = 512;  // 한 번에 버퍼에 쓸 프레임 수

void SpatialAudioPlayer::loadPcmFile(int16_t *pcmData, size_t pcmSize) {
    if (offset < pcmSize) {
        size_t remainingFrames = pcmSize - offset;
        size_t framesToCopy = std::min(framesToWrite, remainingFrames);

        SpatialAudioPlayer::buffer->write(pcmData + offset, framesToCopy);

        offset += framesToCopy;
    }
}

oboe::Result
SpatialAudioPlayer::open(oboe::ChannelMask channelMask, int16_t *pcmData, size_t pcmSize) {
    mDataCallback = std::make_shared<MyDataCallback>(this);
    mErrorCallback = std::make_shared<MyErrorCallback>(this);

    SpatialAudioPlayer::pcmData = pcmData;
    SpatialAudioPlayer::pcmSize = pcmSize;
    SpatialAudioPlayer::offset = 0;

    uint32_t bytesPerFrame = 2 * 1;
    uint32_t capacityInFrames = 1024;
    SpatialAudioPlayer::buffer = std::make_unique<oboe::FifoBuffer>(bytesPerFrame,
                                                                    capacityInFrames);

    AudioStreamBuilder builder;
    oboe::Result result = builder.setSharingMode(oboe::SharingMode::Exclusive)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setFormat(oboe::AudioFormat::I16)
            ->setChannelCount(1)
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
    mParent->loadPcmFile(mParent->pcmData, mParent->pcmSize);
    if (mParent->offset >= mParent->pcmSize) return oboe::DataCallbackResult::Stop;
    buffer->read(audioData, framesToWrite);
    return oboe::DataCallbackResult::Continue;
}

void SpatialAudioPlayer::MyErrorCallback::onErrorAfterClose(oboe::AudioStream *oboeStream,
                                                            oboe::Result error) {
    if (mParent->open(oboeStream->getChannelMask(), mParent->pcmData, mParent->pcmSize) ==
        Result::OK) {
        mParent->start();
    }
}
