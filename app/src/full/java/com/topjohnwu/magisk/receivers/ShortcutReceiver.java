package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ShortcutInfo;
import android.content.pm.ShortcutManager;
import android.graphics.drawable.Icon;
import android.os.Build;

import com.topjohnwu.core.App;
import com.topjohnwu.core.Const;
import com.topjohnwu.core.Data;
import com.topjohnwu.core.utils.Utils;
import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.superuser.Shell;

import java.util.ArrayList;

import androidx.annotation.RequiresApi;

public class ShortcutReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N_MR1) {
            ShortcutManager manager = context.getSystemService(ShortcutManager.class);
            manager.setDynamicShortcuts(getShortCuts(App.self));
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.N_MR1)
    private ArrayList<ShortcutInfo> getShortCuts(App app) {
        ArrayList<ShortcutInfo> shortCuts = new ArrayList<>();
        boolean root = Shell.rootAccess();
        if (Utils.showSuperUser()) {
            shortCuts.add(new ShortcutInfo.Builder(app, "superuser")
                    .setShortLabel(app.getString(R.string.superuser))
                    .setIntent(new Intent(app, ClassMap.get(SplashActivity.class))
                            .putExtra(Const.Key.OPEN_SECTION, "superuser")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(app, R.drawable.sc_superuser))
                    .setRank(0)
                    .build());
        }
        if (root && app.prefs.getBoolean(Const.Key.MAGISKHIDE, false)) {
            shortCuts.add(new ShortcutInfo.Builder(app, "magiskhide")
                    .setShortLabel(app.getString(R.string.magiskhide))
                    .setIntent(new Intent(app, ClassMap.get(SplashActivity.class))
                            .putExtra(Const.Key.OPEN_SECTION, "magiskhide")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(app, R.drawable.sc_magiskhide))
                    .setRank(1)
                    .build());
        }
        if (!app.prefs.getBoolean(Const.Key.COREONLY, false) && root && Data.magiskVersionCode >= 0) {
            shortCuts.add(new ShortcutInfo.Builder(app, "modules")
                    .setShortLabel(app.getString(R.string.modules))
                    .setIntent(new Intent(app, ClassMap.get(SplashActivity.class))
                            .putExtra(Const.Key.OPEN_SECTION, "modules")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(app, R.drawable.sc_extension))
                    .setRank(3)
                    .build());
            shortCuts.add(new ShortcutInfo.Builder(app, "downloads")
                    .setShortLabel(app.getString(R.string.downloads))
                    .setIntent(new Intent(app, ClassMap.get(SplashActivity.class))
                            .putExtra(Const.Key.OPEN_SECTION, "downloads")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(app, R.drawable.sc_cloud_download))
                    .setRank(2)
                    .build());
        }
        return shortCuts;
    }
}
