package com.topjohnwu.magisk.uicomponents;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ShortcutInfo;
import android.content.pm.ShortcutManager;
import android.graphics.drawable.Icon;
import android.os.Build;

import androidx.annotation.RequiresApi;

import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import java.util.ArrayList;

public class Shortcuts {

    public static void setup(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N_MR1) {
            ShortcutManager manager = context.getSystemService(ShortcutManager.class);
            manager.setDynamicShortcuts(getShortCuts(context));
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.N_MR1)
    private static ArrayList<ShortcutInfo> getShortCuts(Context context) {
        ArrayList<ShortcutInfo> shortCuts = new ArrayList<>();
        boolean root = Shell.rootAccess();
        if (Utils.showSuperUser()) {
            shortCuts.add(new ShortcutInfo.Builder(context, "superuser")
                    .setShortLabel(context.getString(R.string.superuser))
                    .setIntent(new Intent(context, ClassMap.get(SplashActivity.class))
                            .putExtra(Const.Key.OPEN_SECTION, "superuser")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(context, R.drawable.sc_superuser))
                    .setRank(0)
                    .build());
        }
        if (root && (boolean) Config.get(Config.Key.MAGISKHIDE)) {
            shortCuts.add(new ShortcutInfo.Builder(context, "magiskhide")
                    .setShortLabel(context.getString(R.string.magiskhide))
                    .setIntent(new Intent(context, ClassMap.get(SplashActivity.class))
                            .putExtra(Const.Key.OPEN_SECTION, "magiskhide")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(context, R.drawable.sc_magiskhide))
                    .setRank(1)
                    .build());
        }
        if (!(boolean) Config.get(Config.Key.COREONLY) && root && Config.magiskVersionCode >= 0) {
            shortCuts.add(new ShortcutInfo.Builder(context, "modules")
                    .setShortLabel(context.getString(R.string.modules))
                    .setIntent(new Intent(context, ClassMap.get(SplashActivity.class))
                            .putExtra(Const.Key.OPEN_SECTION, "modules")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(context, R.drawable.sc_extension))
                    .setRank(3)
                    .build());
            shortCuts.add(new ShortcutInfo.Builder(context, "downloads")
                    .setShortLabel(context.getString(R.string.downloads))
                    .setIntent(new Intent(context, ClassMap.get(SplashActivity.class))
                            .putExtra(Const.Key.OPEN_SECTION, "downloads")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(context, R.drawable.sc_cloud_download))
                    .setRank(2)
                    .build());
        }
        return shortCuts;
    }
}
