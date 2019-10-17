package com.topjohnwu.magisk.view

import android.content.Context
import android.content.Intent
import android.content.pm.ShortcutInfo
import android.content.pm.ShortcutManager
import android.graphics.drawable.Icon
import android.os.Build
import androidx.annotation.RequiresApi
import com.topjohnwu.magisk.*
import com.topjohnwu.magisk.ui.SplashActivity
import com.topjohnwu.magisk.utils.DynAPK
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.superuser.Shell

object Shortcuts {

    fun setup(context: Context) {
        if (Build.VERSION.SDK_INT >= 25) {
            val manager = context.getSystemService(ShortcutManager::class.java)
            manager?.dynamicShortcuts = getShortCuts(context)
        }
    }

    @RequiresApi(api = 25)
    private fun getShortCuts(context: Context): List<ShortcutInfo> {
        val shortCuts = mutableListOf<ShortcutInfo>()
        val root = Shell.rootAccess()
        val intent = context.intent(SplashActivity::class.java)
        if (Utils.showSuperUser()) {
            shortCuts.add(ShortcutInfo.Builder(context, "superuser")
                    .setShortLabel(context.getString(R.string.superuser))
                    .setIntent(Intent(intent)
                            .putExtra(Const.Key.OPEN_SECTION, "superuser")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(context, resolveRes(DynAPK.SUPERUSER)))
                    .setRank(0)
                    .build())
        }
        if (root && Config.magiskHide) {
            shortCuts.add(ShortcutInfo.Builder(context, "magiskhide")
                    .setShortLabel(context.getString(R.string.magiskhide))
                    .setIntent(Intent(intent)
                            .putExtra(Const.Key.OPEN_SECTION, "magiskhide")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(context, resolveRes(DynAPK.MAGISKHIDE)))
                    .setRank(1)
                    .build())
        }
        if (!Config.coreOnly && root && Info.magiskVersionCode >= 0) {
            shortCuts.add(ShortcutInfo.Builder(context, "modules")
                    .setShortLabel(context.getString(R.string.modules))
                    .setIntent(Intent(intent)
                            .putExtra(Const.Key.OPEN_SECTION, "modules")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(context, resolveRes(DynAPK.MODULES)))
                    .setRank(3)
                    .build())
            shortCuts.add(ShortcutInfo.Builder(context, "downloads")
                    .setShortLabel(context.getString(R.string.downloads))
                    .setIntent(Intent(intent)
                            .putExtra(Const.Key.OPEN_SECTION, "downloads")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(context, resolveRes(DynAPK.DOWNLOAD)))
                    .setRank(2)
                    .build())
        }
        return shortCuts
    }
}
