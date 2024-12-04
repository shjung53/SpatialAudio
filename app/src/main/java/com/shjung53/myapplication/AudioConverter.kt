package com.shjung53.myapplication

import android.content.Context
import android.media.MediaCodec
import android.media.MediaExtractor
import android.media.MediaFormat
import android.net.Uri
import android.util.Log
import com.shjung53.audioplayer.AudioPlayer

class AudioConverter : PcmConverter {
    private lateinit var extractor: MediaExtractor

    private fun initializeExtractorFromUri(context: Context, contentUri: Uri): Result<Unit> =
        runCatching {
            extractor = MediaExtractor()
            val assetFileDescriptor =
                context.contentResolver.openAssetFileDescriptor(contentUri, "r")
            assetFileDescriptor?.use {
                extractor.setDataSource(it.fileDescriptor, it.startOffset, it.length)
            }
        }

    private fun selectAudioTrack(): Result<Int> = runCatching {
        for (trackIndex in 0 until extractor.trackCount) {
            val format = extractor.getTrackFormat(trackIndex)
            val mimeType = format.getString(MediaFormat.KEY_MIME)
            if (mimeType?.startsWith("audio/") == true) {
                extractor.selectTrack(trackIndex)
                return Result.success(trackIndex)
            }
        }
        return Result.failure(IllegalStateException("No audio track found in the file"))

    }

    private fun decodeAudioToPCM(audioTrackIndex: Int): Result<Unit> {
        val format = extractor.getTrackFormat(audioTrackIndex)
        val mimeType = format.getString(MediaFormat.KEY_MIME)
        val codec = MediaCodec.createDecoderByType(mimeType!!)
        codec.configure(format, null, null, 0)
        codec.start()

        // PCM 데이터를 저장할 버퍼
        val pcmData = mutableListOf<Byte>()
        val bufferInfo = MediaCodec.BufferInfo()

        while (true) {
            val inputIndex = codec.dequeueInputBuffer(10000)
            if (inputIndex >= 0) {
                val inputByteBuffer = codec.getInputBuffer(inputIndex)!!
                val sampleSize = extractor.readSampleData(inputByteBuffer, 0)
                if (sampleSize < 0) {
                    codec.queueInputBuffer(
                        inputIndex,
                        0,
                        0,
                        0,
                        MediaCodec.BUFFER_FLAG_END_OF_STREAM
                    )
                    break
                } else {
                    codec.queueInputBuffer(inputIndex, 0, sampleSize, extractor.sampleTime, 0)
                    extractor.advance()
                }
            }

            val outputIndex = codec.dequeueOutputBuffer(bufferInfo, 10000)
            if (outputIndex >= 0) {
                val outputBuffer = codec.getOutputBuffer(outputIndex)!!
                val pcmChunk = ByteArray(bufferInfo.size)
                outputBuffer.get(pcmChunk)
                pcmData.addAll(pcmChunk.toList())
                codec.releaseOutputBuffer(outputIndex, false)
            } else if (outputIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                val outputFormat = codec.outputFormat
                val sampleRate = outputFormat.getInteger(MediaFormat.KEY_SAMPLE_RATE)
                val channelCount = outputFormat.getInteger(MediaFormat.KEY_CHANNEL_COUNT)
                Log.d("PCM Info", "Sample Rate: $sampleRate, Channels: $channelCount")
            }
        }
        codec.stop()
        codec.release()
        extractor.release()
//        TODO(재생 분리할 것)
        AudioPlayer().startAudioStream(pcmData.toByteArray())
        return Result.success(Unit)
    }

    override fun audioToPcmData(context: Context, contentUri: Uri): Result<Unit> {
        val initializeResult = initializeExtractorFromUri(context, contentUri)
        if (initializeResult.isFailure) {
            return Result.failure(initializeResult.exceptionOrNull()!!)
        }

        val selectTrackResult = selectAudioTrack()
        if (selectTrackResult.isFailure) {
            return Result.failure(selectTrackResult.exceptionOrNull()!!)
        }

        val decodeResult = decodeAudioToPCM(selectTrackResult.getOrThrow())
        if (decodeResult.isFailure) {
            return Result.failure(decodeResult.exceptionOrNull()!!)
        }

        return Result.success(Unit)
    }
}
