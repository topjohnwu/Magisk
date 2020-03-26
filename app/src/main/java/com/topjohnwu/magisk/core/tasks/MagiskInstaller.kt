package com.topjohnwu.magisk.core.tasks

import android.content.Intent
import android.net.Uri
import android.widget.Toast
import androidx.core.os.postDelayed
import androidx.localbroadcastmanager.content.LocalBroadcastManager
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.utils.Utils
import com.topjohnwu.magisk.extensions.reboot
import com.topjohnwu.magisk.model.events.dialog.EnvFixDialog
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.internal.UiThreadHandler
import java.io.File

sealed class MagiskInstaller(
    file: Uri,
    private val console: MutableList<String>,
    logs: MutableList<String>,
    private val resultListener: FlashResultListener
) : MagiskInstallImpl(file, console, logs) {

    override fun onResult(success: Boolean) {
        if (success) {
            console.add("- All done!")
        } else {
            Shell.sh("rm -rf $installDir").submit()
            console.add("! Installation failed")
        }
        resultListener.onResult(success)
    }

    class Patch(
        file: Uri,
        private val uri: Uri,
        console: MutableList<String>,
        logs: MutableList<String>,
        resultListener: FlashResultListener
    ) : MagiskInstaller(file, console, logs, resultListener) {
        override fun operations() = doPatchFile(uri)
    }

    class SecondSlot(
        file: Uri,
        console: MutableList<String>,
        logs: MutableList<String>,
        resultListener: FlashResultListener
    ) : MagiskInstaller(file, console, logs, resultListener) {
        override fun operations() = secondSlot()
    }

    class Direct(
        file: Uri,
        console: MutableList<String>,
        logs: MutableList<String>,
        resultListener: FlashResultListener
    ) : MagiskInstaller(file, console, logs, resultListener) {
        override fun operations() = direct()
    }

}

class EnvFixTask(
    private val zip: File
) : MagiskInstallImpl() {
    override fun operations() = fixEnv(zip)

    override fun onResult(success: Boolean) {
        LocalBroadcastManager.getInstance(context).sendBroadcast(Intent(EnvFixDialog.DISMISS))
        Utils.toast(
            if (success) R.string.reboot_delay_toast else R.string.setup_fail,
            Toast.LENGTH_LONG
        )
        if (success)
            UiThreadHandler.handler.postDelayed(5000) { reboot() }
    }
}
