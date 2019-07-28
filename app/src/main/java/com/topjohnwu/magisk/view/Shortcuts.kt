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
import com.topjohnwu.magisk.utils.Utils
import com.topjohnwu.superuser.Shell

object Shortcuts {

    fun setup(context: Context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N_MR1) {
            val manager = context.getSystemService(ShortcutManager::class.java)
            manager?.dynamicShortcuts = getShortCuts(context)
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.N_MR1)
    private fun getShortCuts(context: Context): List<ShortcutInfo> {
        val shortCuts = mutableListOf<ShortcutInfo>()
        val root = Shell.rootAccess()
        if (Utils.showSuperUser()) {
            shortCuts.add(ShortcutInfo.Builder(context, "superuser")
                    .setShortLabel(context.getString(R.string.superuser))
                    .setIntent(Intent(context, ClassMap[SplashActivity::class.java])
                            .putExtra(Const.Key.OPEN_SECTION, "superuser")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(context, R.drawable.sc_superuser))
                    .setRank(0)
                    .build())
        }
        if (root && Config.magiskHide) {
            shortCuts.add(ShortcutInfo.Builder(context, "magiskhide")
                    .setShortLabel(context.getString(R.string.magiskhide))
                    .setIntent(Intent(context, ClassMap[SplashActivity::class.java])
                            .putExtra(Const.Key.OPEN_SECTION, "magiskhide")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(context, R.drawable.sc_magiskhide))
                    .setRank(1)
                    .build())
        }
        if (!Config.coreOnly && root && Info.magiskVersionCode >= 0) {
            shortCuts.add(ShortcutInfo.Builder(context, "modules")
                    .setShortLabel(context.getString(R.string.modules))
                    .setIntent(Intent(context, ClassMap[SplashActivity::class.java])
                            .putExtra(Const.Key.OPEN_SECTION, "modules")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(context, R.drawable.sc_extension))
                    .setRank(3)
                    .build())
            shortCuts.add(ShortcutInfo.Builder(context, "downloads")
                    .setShortLabel(context.getString(R.string.downloads))
                    .setIntent(Intent(context, ClassMap[SplashActivity::class.java])
                            .putExtra(Const.Key.OPEN_SECTION, "downloads")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(context, R.drawable.sc_cloud_download))
                    .setRank(2)
                    .build())
        }
        return shortCuts
    }
}
