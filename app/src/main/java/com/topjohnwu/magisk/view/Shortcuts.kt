package com.topjohnwu.magisk.view

import android.content.Context
import android.content.Intent
import android.content.pm.ShortcutInfo
import android.content.pm.ShortcutManager
import android.graphics.drawable.Icon
import android.os.Build
import androidx.annotation.RequiresApi
import androidx.core.content.getSystemService
import androidx.core.graphics.drawable.toAdaptiveIcon
import androidx.core.graphics.drawable.toIcon
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.SplashActivity
import com.topjohnwu.magisk.core.intent
import com.topjohnwu.magisk.ktx.getBitmap
import com.topjohnwu.magisk.utils.Utils

object Shortcuts {

    fun setup(context: Context) {
        if (Build.VERSION.SDK_INT >= 25) {
            val manager = context.getSystemService<ShortcutManager>()
            manager?.dynamicShortcuts =
                getShortCuts(context)
        }
    }

    @RequiresApi(api = 25)
    private fun getShortCuts(context: Context): List<ShortcutInfo> {
        val shortCuts = mutableListOf<ShortcutInfo>()
        val intent = context.intent<SplashActivity>()

        fun getIcon(id: Int): Icon {
            return if (Build.VERSION.SDK_INT >= 26)
                context.getBitmap(id).toAdaptiveIcon()
            else
                context.getBitmap(id).toIcon()
        }

        if (Utils.showSuperUser()) {
            shortCuts.add(
                ShortcutInfo.Builder(context, Const.Nav.SUPERUSER)
                    .setShortLabel(context.getString(R.string.superuser))
                    .setIntent(
                        Intent(intent)
                            .putExtra(Const.Key.OPEN_SECTION, Const.Nav.SUPERUSER)
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK)
                    )
                    .setIcon(getIcon(R.drawable.sc_superuser))
                    .setRank(0)
                    .build()
            )
        }
        if (Info.env.magiskHide) {
            shortCuts.add(
                ShortcutInfo.Builder(context, Const.Nav.HIDE)
                    .setShortLabel(context.getString(R.string.magiskhide))
                    .setIntent(
                        Intent(intent)
                            .putExtra(Const.Key.OPEN_SECTION, Const.Nav.HIDE)
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK)
                    )
                    .setIcon(getIcon(R.drawable.sc_magiskhide))
                    .setRank(1)
                    .build()
            )
        }
        if (Info.env.isActive) {
            shortCuts.add(
                ShortcutInfo.Builder(context, Const.Nav.MODULES)
                    .setShortLabel(context.getString(R.string.modules))
                    .setIntent(
                        Intent(intent)
                            .putExtra(Const.Key.OPEN_SECTION, Const.Nav.MODULES)
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK)
                    )
                    .setIcon(getIcon(R.drawable.sc_extension))
                    .setRank(2)
                    .build()
            )
        }
        return shortCuts
    }
}
