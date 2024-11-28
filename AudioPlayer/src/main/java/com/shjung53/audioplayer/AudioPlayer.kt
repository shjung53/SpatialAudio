package com.shjung53.audioplayer

class AudioPlayer {
    external fun startAudioStream(): Int
    external fun stopAudioStream(): Int

    init {
        System.loadLibrary("AudioPlayer")
    }
}
