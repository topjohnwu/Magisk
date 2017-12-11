package com.topjohnwu.magisk;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.job.JobInfo;
import android.app.job.JobScheduler;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;

import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.asyncs.LoadModules;
import com.topjohnwu.magisk.asyncs.ParallelTask;
import com.topjohnwu.magisk.asyncs.UpdateRepos;
import com.topjohnwu.magisk.components.Activity;
import com.topjohnwu.magisk.services.UpdateCheckService;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.util.List;

public class SplashActivity extends Activity {

    @Override
    public int getDarkTheme() {
        return -1;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        MagiskManager mm = getMagiskManager();

        // Dynamic detect all locales
        new LoadLocale().exec();

        // Create notification channel on Android O
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(Const.ID.NOTIFICATION_CHANNEL,
                    getString(R.string.magisk_updates), NotificationManager.IMPORTANCE_DEFAULT);
            getSystemService(NotificationManager.class).createNotificationChannel(channel);
        }

        mm.loadMagiskInfo();
        Utils.loadPrefs();

        LoadModules loadModuleTask = new LoadModules();

        if (Utils.checkNetworkStatus()) {

            // Fire update check
            new CheckUpdates().exec();

            // Add repo update check
            loadModuleTask.setCallBack(() -> new UpdateRepos(false).exec());
        }

        // Magisk working as expected
        if (Shell.rootAccess() && mm.magiskVersionCode > 0) {

            List<String> ret = Shell.su("echo \"$BOOTIMAGE\"");
            if (Utils.isValidShellResponse(ret)) {
                mm.bootBlock = ret.get(0);
            }

            // Add update checking service
            if (Const.Value.UPDATE_SERVICE_VER > mm.prefs.getInt(Const.Key.UPDATE_SERVICE_VER, -1)) {
                ComponentName service = new ComponentName(this, UpdateCheckService.class);
                JobInfo info = new JobInfo.Builder(Const.ID.UPDATE_SERVICE_ID, service)
                        .setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY)
                        .setPersisted(true)
                        .setPeriodic(8 * 60 * 60 * 1000)
                        .build();
                ((JobScheduler) getSystemService(Context.JOB_SCHEDULER_SERVICE)).schedule(info);
            }

            // Fire asynctasks
            loadModuleTask.exec();

            // Check dtbo status
            Utils.patchDTBO();
        }

        // Write back default values
        mm.prefs.edit()
                .putBoolean(Const.Key.DARK_THEME, mm.isDarkTheme)
                .putBoolean(Const.Key.MAGISKHIDE, mm.magiskHide)
                .putBoolean(Const.Key.UPDATE_NOTIFICATION, mm.updateNotification)
                .putBoolean(Const.Key.HOSTS, Utils.itemExist(Const.MAGISK_HOST_FILE()))
                .putBoolean(Const.Key.COREONLY, Utils.itemExist(Const.MAGISK_DISABLE_FILE))
                .putBoolean(Const.Key.SU_REAUTH, mm.suReauth)
                .putString(Const.Key.SU_REQUEST_TIMEOUT, String.valueOf(mm.suRequestTimeout))
                .putString(Const.Key.SU_AUTO_RESPONSE, String.valueOf(mm.suResponseType))
                .putString(Const.Key.SU_NOTIFICATION, String.valueOf(mm.suNotificationType))
                .putString(Const.Key.ROOT_ACCESS, String.valueOf(mm.suAccessState))
                .putString(Const.Key.SU_MULTIUSER_MODE, String.valueOf(mm.multiuserMode))
                .putString(Const.Key.SU_MNT_NS, String.valueOf(mm.suNamespaceMode))
                .putString(Const.Key.UPDATE_CHANNEL, String.valueOf(mm.updateChannel))
                .putString(Const.Key.LOCALE, mm.localeConfig)
                .putString(Const.Key.BOOT_FORMAT, mm.bootFormat)
                .putInt(Const.Key.UPDATE_SERVICE_VER, Const.Value.UPDATE_SERVICE_VER)
                .apply();

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
