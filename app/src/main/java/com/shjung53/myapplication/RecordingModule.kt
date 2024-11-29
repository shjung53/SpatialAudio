package com.shjung53.myapplication

import android.content.Context
import android.content.pm.PackageManager
import androidx.core.app.ActivityCompat

object RecordingModule {
    fun recordingPermissionCheck(context: Context): Result<Unit> =
        if (ActivityCompat.checkSelfPermission(
                context,
                Manifest.permission.RECORD_AUDIO
            ) == PackageManager.PERMISSION_GRANTED
        ) {
            // 권한이 이미 부여되었을 경우
            Result.success(Unit)
        } else {
            // 권한 요청이 필요한 경우
            ActivityCompat.requestPermissions(
                context as MainActivity,
                arrayOf(
                    Manifest.permission.RECORD_AUDIO,
                    Manifest.permission.READ_EXTERNAL_STORAGE
                ),
                0
            )
            Result.failure(Exception("Permissions not granted"))
        }
}
