package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ShortcutInfo;
import android.content.pm.ShortcutManager;
import android.graphics.drawable.Icon;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.text.TextUtils;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.SplashActivity;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import java.util.ArrayList;

public class ShortcutReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N_MR1) {
            MagiskManager mm = Utils.getMagiskManager(context);
            ShortcutManager manager = context.getSystemService(ShortcutManager.class);
            if (TextUtils.equals(intent.getAction(), Intent.ACTION_LOCALE_CHANGED)) {
                // It is triggered with locale change, manual load Magisk info
                mm.loadMagiskInfo();
            }
            manager.setDynamicShortcuts(getShortCuts(mm));
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.N_MR1)
    private ArrayList<ShortcutInfo> getShortCuts(MagiskManager mm) {
        ArrayList<ShortcutInfo> shortCuts = new ArrayList<>();
        if (Shell.rootAccess() &&
                !(Const.USER_ID > 0 &&
                        mm.multiuserMode == Const.Value.MULTIUSER_MODE_OWNER_MANAGED)) {
            shortCuts.add(new ShortcutInfo.Builder(mm, "superuser")
                    .setShortLabel(mm.getString(R.string.superuser))
                    .setIntent(new Intent(mm, SplashActivity.class)
                            .putExtra(Const.Key.OPEN_SECTION, "superuser")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(mm, R.drawable.sc_superuser))
                    .setRank(0)
                    .build());
        }
        if (Shell.rootAccess() && mm.magiskVersionCode >= Const.MAGISK_VER.UNIFIED
                && mm.prefs.getBoolean(Const.Key.MAGISKHIDE, false)) {
            shortCuts.add(new ShortcutInfo.Builder(mm, "magiskhide")
                    .setShortLabel(mm.getString(R.string.magiskhide))
                    .setIntent(new Intent(mm, SplashActivity.class)
                            .putExtra(Const.Key.OPEN_SECTION, "magiskhide")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(mm, R.drawable.sc_magiskhide))
                    .setRank(1)
                    .build());
        }
        if (!mm.prefs.getBoolean(Const.Key.COREONLY, false) &&
                Shell.rootAccess() && mm.magiskVersionCode >= 0) {
            shortCuts.add(new ShortcutInfo.Builder(mm, "modules")
                    .setShortLabel(mm.getString(R.string.modules))
                    .setIntent(new Intent(mm, SplashActivity.class)
                            .putExtra(Const.Key.OPEN_SECTION, "modules")
                            .setAction(Intent.ACTION_VIEW)
                            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK))
                    .setIcon(Icon.createWithResource(mm, R.drawable.sc_extension))
                    .setRank(3)
                    .build());
            shortCuts.add(new ShortcutInfo.Builder(mm, "downloads")
                    .setShortLabel(mm.getString(R.string.download))
                    .setIntent(new Intent(mm, SplashActivity.class)
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
