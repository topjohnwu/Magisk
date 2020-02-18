package com.topjohnwu.magisk.core.tasks

import android.content.Context
import android.content.ContextWrapper
import android.content.Intent
import android.content.IntentFilter
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.base.BaseReceiver
import com.topjohnwu.magisk.core.view.Notifications
import com.topjohnwu.superuser.Shell
import timber.log.Timber

private const val DTB_PATCH_RESULT = "dtb_result"
private const val DTB_PATCH_ACTION = "com.topjohnwu.magisk.DTBO_PATCH"

private class DTBPatchReceiver : BaseReceiver() {
    override fun onReceive(context: ContextWrapper, intent: Intent?) {
        intent?.also {
            val result = it.getIntExtra(DTB_PATCH_RESULT, 1)
            Timber.d("result=[$result]")
            if (result == 0)
                Notifications.dtboPatched(context)
        }
        context.unregisterReceiver(this)
    }
}

fun patchDTB(context: Context) {
    if (Info.isNewReboot) {
        val c = context.applicationContext
        c.registerReceiver(DTBPatchReceiver(), IntentFilter(DTB_PATCH_ACTION))
        val broadcastCmd = "am broadcast --user ${Const.USER_ID} -p ${c.packageName} " +
                "-a $DTB_PATCH_ACTION --ei $DTB_PATCH_RESULT \$result"
        Shell.su("mm_patch_dtb '$broadcastCmd'").submit()
    }
}
