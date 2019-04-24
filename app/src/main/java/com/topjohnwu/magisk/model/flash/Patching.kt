package com.topjohnwu.magisk.model.flash

import android.net.Uri
import com.topjohnwu.magisk.tasks.MagiskInstaller
import com.topjohnwu.superuser.Shell

sealed class Patching(
    private val console: MutableList<String>,
    logs: MutableList<String>,
    private val resultListener: FlashResultListener
) : MagiskInstaller(console, logs) {

    override fun onResult(success: Boolean) {
        if (success) {
            console.add("- All done!")
        } else {
            Shell.sh("rm -rf $installDir").submit()
            console.add("! Installation failed")
        }
        resultListener.onResult(success)
    }

    class File(
        private val uri: Uri,
        console: MutableList<String>,
        logs: MutableList<String>,
        resultListener: FlashResultListener
    ) : Patching(console, logs, resultListener) {
        override fun operations() =
            extractZip() && handleFile(uri) && patchBoot() && storeBoot()
    }

    class SecondSlot(
        console: MutableList<String>,
        logs: MutableList<String>,
        resultListener: FlashResultListener
    ) : Patching(console, logs, resultListener) {
        override fun operations() =
            findSecondaryImage() && extractZip() && patchBoot() && flashBoot() && postOTA()
    }

    class Direct(
        console: MutableList<String>,
        logs: MutableList<String>,
        resultListener: FlashResultListener
    ) : Patching(console, logs, resultListener) {
        override fun operations() =
            findImage() && extractZip() && patchBoot() && flashBoot()
    }

}