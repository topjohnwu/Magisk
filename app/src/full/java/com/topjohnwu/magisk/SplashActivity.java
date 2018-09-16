package com.topjohnwu.magisk;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;

import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.asyncs.UpdateRepos;
import com.topjohnwu.magisk.components.BaseActivity;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.receivers.ShortcutReceiver;
import com.topjohnwu.magisk.utils.Download;
import com.topjohnwu.magisk.utils.LocaleManager;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

public class SplashActivity extends BaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Magisk working as expected
        if (Shell.rootAccess() && Data.magiskVersionCode > 0) {
            // Update check service
            Utils.setupUpdateCheck();
            // Load modules
            Utils.loadModules();
        }

        mm.repoDB = new RepoDatabaseHelper(this);
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
