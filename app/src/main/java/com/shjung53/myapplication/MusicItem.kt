package com.shjung53.myapplication

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier

@Composable
internal fun MusicItem(name: String, onClickAudioContent: () -> Unit) {
    Row(modifier = Modifier
        .fillMaxSize()
        .background(MaterialTheme.colorScheme.primaryContainer)
        .clickable { onClickAudioContent() }) {
        Text(text = name, color = MaterialTheme.colorScheme.onPrimaryContainer)
    }
}
