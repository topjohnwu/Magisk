package com.topjohnwu.magisk.model.events

import com.topjohnwu.magisk.R
import com.topjohnwu.superuser.Shell
import com.topjohnwu.magisk.extensions.reboot as systemReboot

object RebootEvent {

    @JvmStatic
    fun reboot(menuItemId: Int) = when (menuItemId) {
        R.id.action_reboot_normal -> systemReboot()
        R.id.action_reboot_bootloader -> systemReboot("bootloader")
        R.id.action_reboot_download -> systemReboot("download")
        R.id.action_reboot_edl -> systemReboot("edl")
        R.id.action_reboot_recovery -> Shell.su("/system/bin/reboot recovery").submit()
        else -> Unit
    }

}