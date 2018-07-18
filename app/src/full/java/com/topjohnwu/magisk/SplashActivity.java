package com.topjohnwu.magisk;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;

import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.asyncs.LoadModules;
import com.topjohnwu.magisk.asyncs.ParallelTask;
import com.topjohnwu.magisk.asyncs.UpdateRepos;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.receivers.ShortcutReceiver;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

public class SplashActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        RootUtils.init();
        MagiskManager mm = getMagiskManager();

        mm.repoDB = new RepoDatabaseHelper(this);
        mm.loadMagiskInfo();
        mm.getDefaultInstallFlags();
        mm.loadPrefs();

        // Dynamic detect all locales
        new LoadLocale().exec();

        // Create notification channel on Android O
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(Const.ID.NOTIFICATION_CHANNEL,
                    getString(R.string.magisk_updates), NotificationManager.IMPORTANCE_DEFAULT);
            getSystemService(NotificationManager.class).createNotificationChannel(channel);
        }

        // Setup shortcuts
        sendBroadcast(new Intent(this, ShortcutReceiver.class));

        LoadModules loadModuleTask = new LoadModules();

        if (Utils.checkNetworkStatus()) {
            // Fire update check
            new CheckUpdates().exec();
            // Add repo update check
            loadModuleTask.setCallBack(() -> new UpdateRepos(false).exec());
        }

        // Magisk working as expected
        if (Shell.rootAccess() && mm.magiskVersionCode > 0) {
            // Update check service
            mm.setupUpdateCheck();
            // Fire asynctasks
            loadModuleTask.exec();
        }

        // Write back default values
        mm.writeConfig();

        mm.hasInit = true;

        Intent intent = new Intent(this, MainActivity.class);
        intent.putExtra(Const.Key.OPEN_SECTION, getIntent().getStringExtra(Const.Key.OPEN_SECTION));
        intent.putExtra(Const.Key.INTENT_PERM, getIntent().getStringExtra(Const.Key.INTENT_PERM));
        startActivity(intent);
        finish();
    }

    static class LoadLocale extends ParallelTask<Void, Void, Void> {
        @Override
        protected Void doInBackground(Void... voids) {
            MagiskManager.get().locales = Utils.getAvailableLocale();
            return null;
        }
        @Override
        protected void onPostExecute(Void aVoid) {
            MagiskManager.get().localeDone.publish();
        }
    }
}
