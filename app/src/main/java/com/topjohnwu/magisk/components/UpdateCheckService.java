package com.topjohnwu.magisk.components;

import androidx.annotation.NonNull;
import androidx.work.ListenableWorker;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.tasks.CheckUpdates;
import com.topjohnwu.magisk.uicomponents.Notifications;
import com.topjohnwu.superuser.Shell;

public class UpdateCheckService extends DelegateWorker {

    @NonNull
    @Override
    public ListenableWorker.Result doWork() {
        if (App.foreground() == null) {
            Shell.getShell();
            CheckUpdates.check(this::onCheckDone);
        }
        return ListenableWorker.Result.success();
    }

    private void onCheckDone() {
        if (BuildConfig.VERSION_CODE < Config.remoteManagerVersionCode) {
            Notifications.managerUpdate();
        } else if (Config.magiskVersionCode < Config.remoteMagiskVersionCode) {
            Notifications.magiskUpdate();
        }
    }
}
