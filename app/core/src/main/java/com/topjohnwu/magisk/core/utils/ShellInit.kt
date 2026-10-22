package com.topjohnwu.magisk.core.utils

import android.content.Context
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.superuser.Shell
import java.io.File

class ShellInit : Shell.Initializer() {
    override fun onInit(context: Context, shell: Shell): Boolean {
        if (shell.isRoot) {
            Info.isRooted = true
            RootUtils.bindTask?.let { shell.execTask(it) }
            RootUtils.bindTask = null
        }
        shell.newJob().apply {
            add("export ASH_STANDALONE=1")

            val localBB = File(context.applicationInfo.nativeLibraryDir, "libbusybox.so")

            if (shell.isRoot) {
                add("export MAGISKTMP=\$(magisk --path)")
                // Test if we can properly execute stuff in /data
                Info.noDataExec = !shell.newJob()
                    .add("$localBB sh -c '$localBB true'").exec().isSuccess
            }

            if (Info.noDataExec) {
                // Copy it out of /data to workaround Samsung bullshit
                add(
                    "if [ -x \$MAGISKTMP/.magisk/busybox/busybox ]; then",
                    "  cp -af $localBB \$MAGISKTMP/.magisk/busybox/busybox",
                    "  exec \$MAGISKTMP/.magisk/busybox/busybox sh",
                    "else",
                    "  cp -af $localBB /dev/busybox",
                    "  exec /dev/busybox sh",
                    "fi"
                )
            } else {
                // Directly execute the file
                add("exec $localBB sh")
            }

            add(context.assets.open("app_functions.sh"))
            if (shell.isRoot) {
                add(context.assets.open("util_functions.sh"))
            }
        }.exec()

        Info.init(shell)
        return true
    }
}
