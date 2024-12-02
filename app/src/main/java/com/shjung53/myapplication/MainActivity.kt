package com.shjung53.myapplication

import android.content.ContentUris
import android.content.pm.PackageManager
import android.media.MediaCodec
import android.media.MediaExtractor
import android.media.MediaFormat
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.provider.MediaStore
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import androidx.compose.ui.tooling.preview.Preview
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.shjung53.audioplayer.AudioPlayer
import com.shjung53.myapplication.ui.theme.MyApplicationTheme


class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

// Register the permissions callback, which handles the user's response to the
// system permissions dialog. Save the return value, an instance of
// ActivityResultLauncher. You can use either a val, as shown in this snippet,
// or a lateinit var in your onAttach() or onCreate() method.
        val requestPermissionLauncher =
            registerForActivityResult(
                ActivityResultContracts.RequestPermission()
            ) { isGranted: Boolean ->
                if (isGranted) {
                    // Permission is granted. Continue the action or workflow in your
                    // app.
                } else {
                    // Explain to the user that the feature is unavailable because the
                    // feature requires a permission that the user has denied. At the
                    // same time, respect the user's decision. Don't link to system
                    // settings in an effort to convince the user to change their
                    // decision.
                }
            }

        when {
            ContextCompat.checkSelfPermission(
                this,
                android.Manifest.permission.READ_MEDIA_AUDIO
            ) == PackageManager.PERMISSION_GRANTED -> {
                // You can use the API that requires the permission.
            }

            ActivityCompat.shouldShowRequestPermissionRationale(
                this, android.Manifest.permission.READ_MEDIA_AUDIO
            ) -> {
//                TODO(권한 설정 설명)
            }

            else -> {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                    requestPermissionLauncher.launch(
                        android.Manifest.permission.READ_MEDIA_AUDIO
                    )
                }
            }
        }

        val projection = arrayOf(
            MediaStore.Audio.Media._ID,
            MediaStore.Audio.Media.DISPLAY_NAME, // 파일 이름
            MediaStore.Audio.Media.DATE_ADDED   // 추가된 날짜
        )

        val selection = "${MediaStore.Audio.Media.MIME_TYPE} = ?"
        val selectionArgs = arrayOf("audio/mp4")

        val sortOrder = "${MediaStore.Audio.Media.DATE_ADDED} DESC" // 최신 파일 순으로 정렬

        val cursor = contentResolver.query(
            MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
            projection,
            selection,
            selectionArgs,
            sortOrder
        )

        val recordingFiles = mutableListOf<Uri>()

        cursor?.use {
            val idColumn = it.getColumnIndexOrThrow(MediaStore.Audio.Media._ID)
            val nameColumn = it.getColumnIndexOrThrow(MediaStore.Audio.Media.DISPLAY_NAME)

            while (it.moveToNext()) {
                val id = it.getLong(idColumn)
                val displayName = it.getString(nameColumn)

                // 파일의 URI 생성
                val contentUri = ContentUris.withAppendedId(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, id)

                // 경로 대신 URI 저장
                recordingFiles.add(contentUri)

                Log.d("RecordingFile", "ID: $id, Name: $displayName, URI: $contentUri")
            }
        }

// 녹음 파일 URI 출력
        recordingFiles.forEach { uri ->
            Log.d("RecordingFile", "Recording File URI: $uri")
        }


        val extractor = MediaExtractor()
        try {
            // ContentResolver에서 AssetFileDescriptor를 가져옴
            val afd = contentResolver.openAssetFileDescriptor(recordingFiles[0], "r")
            afd?.use {
                extractor.setDataSource(it.fileDescriptor, it.startOffset, it.length)
                Log.d("MediaExtractor", "Extractor initialized successfully")
            }
        } catch (e: Exception) {
            Log.e("MediaExtractor", "Failed to set data source: ${e.message}")
        }

        var audioTrackIndex = -1
        for (i in 0 until extractor.trackCount) {
            val format = extractor.getTrackFormat(i)
            val mimeType = format.getString(MediaFormat.KEY_MIME)
            if (mimeType?.startsWith("audio/") == true) {
                audioTrackIndex = i
                extractor.selectTrack(i) // 오디오 트랙 선택
                break
            }
        }

        if (audioTrackIndex == -1) {
            throw IllegalStateException("No audio track found in the file")
        }

        // MediaCodec 초기화
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

// MediaCodec과 MediaExtractor 종료
        codec.stop()
        codec.release()
        extractor.release()

        AudioPlayer().startAudioStream(pcmData.toByteArray())

        setContent {
            MyApplicationTheme {
                PlayList()
            }
        }
    }

    @Preview(showBackground = true)
    @Composable
    fun GreetingPreview() {
        MyApplicationTheme {
            PlayList()
        }
    }
}
