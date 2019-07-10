package com.topjohnwu.magisk.view.dialogs

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.model.download.CompoundDownloadService
import com.topjohnwu.magisk.model.entity.internal.Configuration
import com.topjohnwu.magisk.model.entity.internal.DownloadSubject
import com.topjohnwu.magisk.ui.base.MagiskActivity
import com.topjohnwu.magisk.utils.Utils

internal class InstallMethodDialog(activity: MagiskActivity<*, *>, options: List<String>) :
    AlertDialog.Builder(activity) {

    init {
        setTitle(R.string.select_method)
        setItems(options.toTypedArray()) { _, idx ->
            when (idx) {
                0 -> downloadOnly(activity)
                1 -> patchBoot(activity)
                2 -> flash(activity)
                3 -> installInactiveSlot(activity)
            }
        }
    }

    @SuppressLint("MissingPermission")
    private fun flash(activity: MagiskActivity<*, *>) = activity.withExternalRW {
        onSuccess {
            val subject = DownloadSubject.Magisk(Configuration.Flash.Primary)
            CompoundDownloadService.download(context, subject)
        }
    }

    @SuppressLint("MissingPermission")
    private fun patchBoot(activity: MagiskActivity<*, *>) = activity.withExternalRW {
        onSuccess {
            Utils.toast(R.string.patch_file_msg, Toast.LENGTH_LONG)
            val intent = Intent(Intent.ACTION_GET_CONTENT)
                .setType("*/*")
                .addCategory(Intent.CATEGORY_OPENABLE)
            activity.startActivityForResult(intent, Const.ID.SELECT_BOOT) { resultCode, data ->
                if (resultCode == Activity.RESULT_OK && data != null) {
                    val safeData = data.data ?: Uri.EMPTY
                    val subject = DownloadSubject.Magisk(Configuration.Patch(safeData))
                    CompoundDownloadService.download(activity, subject)
                }
            }
        }
    }

    @SuppressLint("MissingPermission")
    private fun downloadOnly(activity: MagiskActivity<*, *>) = activity.withExternalRW {
        onSuccess {
            val subject = DownloadSubject.Magisk(Configuration.Download)
            CompoundDownloadService.download(activity, subject)
        }
    }

    @SuppressLint("MissingPermission")
    private fun installInactiveSlot(activity: MagiskActivity<*, *>) {
        CustomAlertDialog(activity)
            .setTitle(R.string.warning)
            .setMessage(R.string.install_inactive_slot_msg)
            .setCancelable(true)
            .setPositiveButton(R.string.yes) { _, _ ->
                val subject = DownloadSubject.Magisk(Configuration.Flash.Secondary)
                CompoundDownloadService.download(activity, subject)
            }
            .setNegativeButton(R.string.no_thanks, null)
            .show()
    }
}
