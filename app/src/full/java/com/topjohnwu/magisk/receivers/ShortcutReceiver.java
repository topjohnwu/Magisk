package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ShortcutInfo;
import android.content.pm.ShortcutManager;
import android.graphics.drawable.Icon;
import android.os.Build;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import java.util.ArrayList;

import androidx.annotation.RequiresApi;

public class ShortcutReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N_MR1) {
            MagiskManager mm = Data.MM();
            ShortcutManager manager = context.getSystemService(ShortcutManager.class);
            manager.setDynamicShortcuts(getShortCuts(mm));
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.N_MR1)
    private ArrayList<ShortcutInfo> getShortCuts(MagiskManager mm) {
        ArrayList<ShortcutInfo> shortCuts = new ArrayList<>();
        boolean root = Shell.rootAccess();
        if (Utils.showSuperUser()) {
            shortCuts.add(new ShortcutInfo.Builder(mm, "superuser")
                    .setShortLabel(mm.getString(R.string.superuser))
                    .setIntent(new Intent(mm, Data.classMap.get(SplashActivity.class))
                            .putExtra(Const.Key.OPEN_SECTION, "superuser")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(mm, R.drawable.sc_superuser))
                    .setRank(0)
                    .build());
        }
        if (root && mm.prefs.getBoolean(Const.Key.MAGISKHIDE, false)) {
            shortCuts.add(new ShortcutInfo.Builder(mm, "magiskhide")
                    .setShortLabel(mm.getString(R.string.magiskhide))
                    .setIntent(new Intent(mm, Data.classMap.get(SplashActivity.class))
                            .putExtra(Const.Key.OPEN_SECTION, "magiskhide")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(mm, R.drawable.sc_magiskhide))
                    .setRank(1)
                    .build());
        }
        if (!mm.prefs.getBoolean(Const.Key.COREONLY, false) && root && Data.magiskVersionCode >= 0) {
            shortCuts.add(new ShortcutInfo.Builder(mm, "modules")
                    .setShortLabel(mm.getString(R.string.modules))
                    .setIntent(new Intent(mm, Data.classMap.get(SplashActivity.class))
                            .putExtra(Const.Key.OPEN_SECTION, "modules")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(mm, R.drawable.sc_extension))
                    .setRank(3)
                    .build());
            shortCuts.add(new ShortcutInfo.Builder(mm, "downloads")
                    .setShortLabel(mm.getString(R.string.downloads))
                    .setIntent(new Intent(mm, Data.classMap.get(SplashActivity.class))
                            .putExtra(Const.Key.OPEN_SECTION, "downloads")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(mm, R.drawable.sc_cloud_download))
                    .setRank(2)
                    .build());
        }
        return shortCuts;
    }
}
