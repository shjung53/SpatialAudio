package com.shjung53.myapplication

import android.content.Context
import android.net.Uri

interface PcmConverter {
    fun audioToPcmData(context: Context, contentUri: Uri): Result<Unit>
}
