package com.shjung53.myapplication

import android.content.ContentUris
import android.provider.MediaStore
import androidx.compose.foundation.layout.systemBarsPadding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext

@Composable
internal fun PlayList() {
    val projection = arrayOf(
        MediaStore.Audio.Media._ID,
        MediaStore.Audio.Media.DISPLAY_NAME, // 파일 이름
        MediaStore.Audio.Media.DATE_ADDED   // 추가된 날짜
    )
    val context = LocalContext.current

    val selection = "${MediaStore.Audio.Media.MIME_TYPE} = ?"
    val selectionArgs = arrayOf("audio/mpeg")
    val sortOrder = "${MediaStore.Audio.Media.DATE_ADDED} DESC" // 최신 파일 순으로 정렬

    val cursor = context.contentResolver.query(
        MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
        projection,
        selection,
        selectionArgs,
        sortOrder
    )

    val audioContentList = mutableListOf<AudioContent>()

    cursor?.use {
        val idColumn = it.getColumnIndexOrThrow(MediaStore.Audio.Media._ID)
        val nameColumn = it.getColumnIndexOrThrow(MediaStore.Audio.Media.DISPLAY_NAME)

        while (it.moveToNext()) {
            val id = it.getLong(idColumn)
            val displayName = it.getString(nameColumn)

            // 파일의 URI 생성
            val contentUri =
                ContentUris.withAppendedId(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, id)

            // 경로 대신 URI 저장
            audioContentList.add(AudioContent(displayName, contentUri))
        }
    }

    val audioConverter = AudioConverter()

    LazyColumn(modifier = Modifier.systemBarsPadding()) {
        items(audioContentList) { audioContent ->
            MusicItem(audioContent.name) {
                audioConverter.audioToPcmData(
                    context,
                    audioContent.uri
                )
            }
        }
    }
}
