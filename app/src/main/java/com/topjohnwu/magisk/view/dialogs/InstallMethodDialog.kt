package com.topjohnwu.magisk.view.dialogs

import android.app.Activity
import android.content.Intent
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import com.google.android.material.snackbar.Snackbar
import com.topjohnwu.magisk.ClassMap
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.Info
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.ui.base.MagiskActivity
import com.topjohnwu.magisk.ui.flash.FlashActivity
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.magisk.view.ProgressNotification
import com.topjohnwu.magisk.view.SnackbarMaker
import com.topjohnwu.net.Networking
import java.io.File

internal class InstallMethodDialog(activity: MagiskActivity<*, *>, options: List<String>) : AlertDialog.Builder(activity) {

    init {
        setTitle(R.string.select_method)
        setItems(options.toTypedArray()) { _, idx ->
            when (idx) {
                0 -> downloadOnly(activity)
                1 -> patchBoot(activity)
                2 -> {
                    val intent = Intent(activity, ClassMap[FlashActivity::class.java])
                            .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_MAGISK)
                    activity.startActivity(intent)
                }
                3 -> installInactiveSlot(activity)
            }
        }
    }

    private fun patchBoot(activity: MagiskActivity<*, *>) {
        activity.withExternalRW {
            onSuccess {
                Utils.toast(R.string.patch_file_msg, Toast.LENGTH_LONG)
                val intent = Intent(Intent.ACTION_GET_CONTENT)
                        .setType("*/*")
                        .addCategory(Intent.CATEGORY_OPENABLE)
                activity.startActivityForResult(intent, Const.ID.SELECT_BOOT) { resultCode, data ->
                    if (resultCode == Activity.RESULT_OK && data != null) {
                        val i = Intent(this, ClassMap[FlashActivity::class.java])
                                .setData(data.data)
                                .putExtra(Const.Key.FLASH_ACTION, Const.Value.PATCH_FILE)
                        startActivity(i)
                    }
                }
            }
        }
    }

    private fun downloadOnly(activity: MagiskActivity<*, *>) {
        activity.withExternalRW {
            onSuccess {
                val filename = "Magisk-v${Info.remote.magisk.version}" +
                        "(${Info.remote.magisk.versionCode}).zip"
                val zip = File(Const.EXTERNAL_PATH, filename)
                val progress = ProgressNotification(filename)
                Networking.get(Info.remote.magisk.link)
                        .setDownloadProgressListener(progress)
                        .setErrorHandler { _, _ -> progress.dlFail() }
                        .getAsFile(zip) {
                            progress.dlDone()
                            SnackbarMaker.make(activity,
                                    activity.getString(R.string.internal_storage, "/Download/$filename"),
                                    Snackbar.LENGTH_LONG).show()
                        }
            }
        }
    }

    private fun installInactiveSlot(activity: MagiskActivity<*, *>) {
        CustomAlertDialog(activity)
                .setTitle(R.string.warning)
                .setMessage(R.string.install_inactive_slot_msg)
                .setCancelable(true)
                .setPositiveButton(R.string.yes) { _, _ ->
                    val intent = Intent(activity, ClassMap[FlashActivity::class.java])
                            .putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_INACTIVE_SLOT)
                    activity.startActivity(intent)
                }
                .setNegativeButton(R.string.no_thanks, null)
                .show()
    }
}
