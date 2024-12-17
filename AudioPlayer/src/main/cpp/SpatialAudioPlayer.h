//
// Created by shjung on 2024-11-29.
//

#ifndef MY_APPLICATION_SPATIALAUDIOPLAYER_H
#define MY_APPLICATION_SPATIALAUDIOPLAYER_H

#include "oboe/Oboe.h"

class SpatialAudioPlayer {

private:
    static std::unique_ptr<oboe::FifoBuffer> buffer;
    int16_t *pcmData;
    size_t offset = 0;
    size_t pcmSize = 0;

public:
    void loadPcmFile(int16_t *pcmData, size_t pcmSize);

    oboe::Result open(oboe::ChannelMask, int16_t *pcmData, size_t pcmSize);

    oboe::Result start();

    oboe::Result stop();

    oboe::Result close();

private:
    class MyDataCallback : public oboe::AudioStreamDataCallback {
public: MyDataCallback(SpatialAudioPlayer *parent): mParent(parent) {}

    public:
        oboe::DataCallbackResult onAudioReady(
                oboe::AudioStream *audioStream,
                void *audioData,
                int32_t numFrames) override;

    private:
        SpatialAudioPlayer *mParent;
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
    std::shared_ptr<MyDataCallback> mDataCallback;
    std::shared_ptr<MyErrorCallback> mErrorCallback;
};


#endif //MY_APPLICATION_SPATIALAUDIOPLAYER_H
