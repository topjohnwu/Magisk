package com.topjohnwu.magisk.services;

import com.topjohnwu.core.Data;
import com.topjohnwu.core.tasks.CheckUpdates;
import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.components.Notifications;
import com.topjohnwu.superuser.Shell;

import androidx.annotation.NonNull;
import androidx.work.ListenableWorker;

public class UpdateCheckService extends DelegateWorker {

    @NonNull
    @Override
    public ListenableWorker.Result doWork() {
        Shell.getShell();
        CheckUpdates.checkNow(this::onCheckDone);
        return ListenableWorker.Result.success();
    }

    private void onCheckDone() {
        if (BuildConfig.VERSION_CODE < Data.remoteManagerVersionCode) {
            Notifications.managerUpdate();
        } else if (Data.magiskVersionCode < Data.remoteMagiskVersionCode) {
            Notifications.magiskUpdate();
        }
    }
}
