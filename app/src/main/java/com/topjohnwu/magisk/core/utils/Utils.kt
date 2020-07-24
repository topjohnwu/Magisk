package com.topjohnwu.magisk.core.utils

import android.content.Context
import android.content.Intent
import android.content.res.Resources
import android.net.Uri
import android.os.Environment
import android.widget.Toast
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Config
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.ktx.get
import com.topjohnwu.superuser.internal.UiThreadHandler
import java.io.File

object Utils {

    fun toast(msg: CharSequence, duration: Int) {
        UiThreadHandler.run { Toast.makeText(get(), msg, duration).show() }
    }

    fun toast(resId: Int, duration: Int) {
        UiThreadHandler.run { Toast.makeText(get(), resId, duration).show() }
    }

    fun dpInPx(dp: Int): Int {
        val scale = get<Resources>().displayMetrics.density
        return (dp * scale + 0.5).toInt()
    }

    fun showSuperUser(): Boolean {
        return Info.env.isActive && (Const.USER_ID == 0
                || Config.suMultiuserMode != Config.Value.MULTIUSER_MODE_OWNER_MANAGED)
    }

    fun openLink(context: Context, link: Uri) {
        val intent = Intent(Intent.ACTION_VIEW, link)
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
        if (intent.resolveActivity(context.packageManager) != null) {
            context.startActivity(intent)
        } else {
            toast(
                R.string.open_link_failed_toast,
                Toast.LENGTH_SHORT
            )
        }
    }

    fun ensureDownloadPath(path: String) =
        File(Environment.getExternalStorageDirectory(), path).run {
            if ((exists() && isDirectory) || mkdirs()) this else null
        }

}
