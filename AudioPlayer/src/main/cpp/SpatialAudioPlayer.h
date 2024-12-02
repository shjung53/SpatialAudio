//
// Created by shjung on 2024-11-29.
//

#ifndef MY_APPLICATION_SPATIALAUDIOPLAYER_H
#define MY_APPLICATION_SPATIALAUDIOPLAYER_H

#include "oboe/Oboe.h"

class SpatialAudioPlayer {

public:
    static std::vector<uint8_t> mPcmData;

public:
    oboe::Result open(oboe::ChannelMask, std::vector<uint8_t>);

    oboe::Result start();

    oboe::Result stop();

    oboe::Result close();

private:
    class MyPcmLoadCallback : public oboe::AudioStreamDataCallback {
    public:
        oboe::DataCallbackResult onAudioReady(
                oboe::AudioStream *audioStream,
                void *audioData,
                int32_t numFrames) override;
    };

    class MyDataCallback : public oboe::AudioStreamDataCallback {
    public:
        oboe::DataCallbackResult onAudioReady(
                oboe::AudioStream *audioStream,
                void *audioData,
                int32_t numFrames) override;
    };

    class MyErrorCallback : public oboe::AudioStreamErrorCallback {
    public:
        MyErrorCallback(SpatialAudioPlayer *parent) : mParent(parent) {}

        virtual ~MyErrorCallback() {
        }

        void onErrorAfterClose(oboe::AudioStream *oboeStream, oboe::Result error) override;

    private:
        SpatialAudioPlayer *mParent;
    };

    std::shared_ptr<oboe::AudioStream> mStream;
    std::shared_ptr<MyPcmLoadCallback> mPcmLoadCallback;
    std::shared_ptr<MyDataCallback> mDataCallback;
    std::shared_ptr<MyErrorCallback> mErrorCallback;

    static constexpr int kChannelCount = 7;
};


#endif //MY_APPLICATION_SPATIALAUDIOPLAYER_H
