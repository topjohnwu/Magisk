package com.topjohnwu.magisk.utils

import android.content.Context
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.extensions.rawResource
import com.topjohnwu.magisk.wrap
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.io.SuFile

class RootInit : Shell.Initializer() {

    override fun onInit(context: Context, shell: Shell): Boolean {
        return init(context.wrap(), shell)
    }

    fun init(context: Context, shell: Shell): Boolean {
        val job = shell.newJob()
        if (shell.isRoot) {
            job.add(context.rawResource(R.raw.util_functions))
                .add(context.rawResource(R.raw.utils))
            Const.MAGISK_DISABLE_FILE = SuFile("/cache/.disable_magisk")
        } else {
            job.add(context.rawResource(R.raw.nonroot_utils))
        }

        job.add(
            "mount_partitions",
            "get_flags",
            "run_migrations",
            "export BOOTMODE=true"
        ).exec()

        return true
    }
}
