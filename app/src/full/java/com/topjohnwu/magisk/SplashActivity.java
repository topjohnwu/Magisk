package com.topjohnwu.magisk;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.text.TextUtils;

import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.asyncs.UpdateRepos;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.receivers.ShortcutReceiver;
import com.topjohnwu.magisk.utils.Download;
import com.topjohnwu.magisk.utils.LocaleManager;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

public class SplashActivity extends BaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        String pkg = mm.mDB.getStrings(Const.Key.SU_MANAGER, null);
        if (pkg != null && getPackageName().equals(Const.ORIG_PKG_NAME)) {
            mm.mDB.setStrings(Const.Key.SU_MANAGER, null);
            Shell.su("pm uninstall " + pkg).exec();
        }
        if (TextUtils.equals(pkg, getPackageName())) {
            try {
                // We are the manager, remove com.topjohnwu.magisk as it could be malware
                getPackageManager().getApplicationInfo(Const.ORIG_PKG_NAME, 0);
                RootUtils.uninstallPkg(Const.ORIG_PKG_NAME);
            } catch (PackageManager.NameNotFoundException ignored) {}
        }

        // Magisk working as expected
        if (Shell.rootAccess() && Data.magiskVersionCode > 0) {
            // Update check service
            Utils.setupUpdateCheck();
            // Load modules
            Utils.loadModules();
        }

        Data.importPrefs();

        // Dynamic detect all locales
        LocaleManager.loadAvailableLocales();

        // Create notification channel on Android O
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(Const.ID.NOTIFICATION_CHANNEL,
                    getString(R.string.magisk_updates), NotificationManager.IMPORTANCE_DEFAULT);
            getSystemService(NotificationManager.class).createNotificationChannel(channel);
        }

        // Setup shortcuts
        sendBroadcast(new Intent(this, Data.classMap.get(ShortcutReceiver.class)));

        if (Download.checkNetworkStatus(this)) {
            // Fire update check
            CheckUpdates.check();
            // Repo update check
            new UpdateRepos().exec();
        }

        // Write back default values
        Data.writeConfig();

        mm.hasInit = true;

        Intent intent = new Intent(this, Data.classMap.get(MainActivity.class));
        intent.putExtra(Const.Key.OPEN_SECTION, getIntent().getStringExtra(Const.Key.OPEN_SECTION));
        intent.putExtra(BaseActivity.INTENT_PERM, getIntent().getStringExtra(BaseActivity.INTENT_PERM));
        startActivity(intent);
        finish();
    }
}
