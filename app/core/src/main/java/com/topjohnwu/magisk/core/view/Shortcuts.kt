package com.topjohnwu.magisk.view

import android.content.Context
import android.content.Intent
import android.content.pm.ShortcutInfo
import android.content.pm.ShortcutManager
import android.graphics.drawable.Icon
import android.os.Build
import androidx.annotation.RequiresApi
import androidx.core.content.getSystemService
import androidx.core.content.pm.ShortcutInfoCompat
import androidx.core.content.pm.ShortcutManagerCompat
import androidx.core.graphics.drawable.IconCompat
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.core.isRunningAsStub
import com.topjohnwu.magisk.core.ktx.getBitmap

object Shortcuts {

    fun setupDynamic(context: Context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N_MR1) {
            val manager = context.getSystemService<ShortcutManager>() ?: return
            manager.dynamicShortcuts = getShortCuts(context)
        }
    }

    fun addHomeIcon(context: Context) {
        val intent = context.packageManager.getLaunchIntentForPackage(context.packageName) ?: return
        val info = ShortcutInfoCompat.Builder(context, Const.Nav.HOME)
            .setShortLabel(context.getString(R.string.magisk))
            .setIntent(intent)
            .setIcon(context.getIconCompat(R.drawable.ic_launcher))
            .build()
        ShortcutManagerCompat.requestPinShortcut(context, info, null)
    }

    private fun Context.getIcon(id: Int): Icon {
        return if (isRunningAsStub) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
                Icon.createWithAdaptiveBitmap(getBitmap(id))
            else
                Icon.createWithBitmap(getBitmap(id))
        } else {
            Icon.createWithResource(this, id)
        }
    }

    private fun Context.getIconCompat(id: Int): IconCompat {
        return if (isRunningAsStub) {
            val bitmap = getBitmap(id)
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
                IconCompat.createWithAdaptiveBitmap(bitmap)
            else
                IconCompat.createWithBitmap(bitmap)
        } else {
            IconCompat.createWithResource(this, id)
        }
    }

    @RequiresApi(api = 25)
    private fun getShortCuts(context: Context): List<ShortcutInfo> {
        val intent = context.packageManager.getLaunchIntentForPackage(context.packageName)
            ?: return emptyList()

        val shortCuts = mutableListOf<ShortcutInfo>()

        if (Info.showSuperUser) {
            shortCuts.add(
                ShortcutInfo.Builder(context, Const.Nav.SUPERUSER)
                    .setShortLabel(context.getString(R.string.superuser))
                    .setIntent(
                        Intent(intent).putExtra(Const.Key.OPEN_SECTION, Const.Nav.SUPERUSER)
                    )
                    .setIcon(context.getIcon(R.drawable.sc_superuser))
                    .setRank(0)
                    .build()
            )
        }
        if (Info.env.isActive) {
            shortCuts.add(
                ShortcutInfo.Builder(context, Const.Nav.MODULES)
                    .setShortLabel(context.getString(R.string.modules))
                    .setIntent(
                        Intent(intent).putExtra(Const.Key.OPEN_SECTION, Const.Nav.MODULES)
                    )
                    .setIcon(context.getIcon(R.drawable.sc_extension))
                    .setRank(1)
                    .build()
            )
        }
        return shortCuts
    }
}
