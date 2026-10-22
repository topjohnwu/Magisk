package com.topjohnwu.magisk.ui.flash

import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.net.Uri
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.cmp
import com.topjohnwu.magisk.ui.MainActivity
import java.util.UUID

object FlashUtils {

    const val INTENT_FLASH = "com.topjohnwu.magisk.intent.FLASH"
    const val EXTRA_FLASH_ACTION = "flash_action"
    const val EXTRA_FLASH_URI = "flash_uri"
    private const val EXTRA_FLASH_TOKEN = "flash_token"

    // Random per-process token attached to our own install intents. Other apps cannot read
    // the extras of our PendingIntents, so a matching token proves the request came from us.
    private val token = UUID.randomUUID().toString()

    fun isTrusted(intent: Intent) = intent.getStringExtra(EXTRA_FLASH_TOKEN) == token

    /**
     * Creates the intent to install a downloaded module. [requestCode] must be unique per
     * download (e.g. its notification ID), so re-downloads of the same file each get their
     * own one-shot PendingIntent.
     */
    fun installIntent(context: Context, file: Uri, requestCode: Int): PendingIntent {
        val intent = Intent(context, MainActivity::class.java).apply {
            component = MainActivity::class.java.cmp(context.packageName)
            action = INTENT_FLASH
            putExtra(EXTRA_FLASH_ACTION, Const.Value.FLASH_ZIP)
            putExtra(EXTRA_FLASH_URI, file.toString())
            putExtra(EXTRA_FLASH_TOKEN, token)
            flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP or Intent.FLAG_ACTIVITY_SINGLE_TOP
        }
        // One-shot: it skips the install confirmation, so whoever holds it (e.g. a notification
        // listener) must not be able to send it again
        return PendingIntent.getActivity(
            context, requestCode, intent,
            PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_ONE_SHOT
        )
    }
}
