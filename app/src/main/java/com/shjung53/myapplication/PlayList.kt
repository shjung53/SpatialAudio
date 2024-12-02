package com.shjung53.myapplication

import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.items
import androidx.compose.runtime.Composable

@Composable
internal fun PlayList() {
    LazyRow {
        items((1..10).toList()) {
            MusicItem()
        }
    }
}
