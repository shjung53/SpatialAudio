package com.shjung53.audioplayer

class AudioPlayer {
    external fun startAudioStream(pcmData: ByteArray): Int
    external fun stopAudioStream(): Int

    init {
        System.loadLibrary("AudioPlayer")
    }
}
