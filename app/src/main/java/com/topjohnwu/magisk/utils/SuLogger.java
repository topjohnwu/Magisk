package com.topjohnwu.magisk.utils;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Process;
import android.widget.Toast;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.container.Policy;
import com.topjohnwu.magisk.container.SuLogEntry;

import java.util.Date;

public abstract class SuLogger {

    public void handleLogs(Intent intent) {

        int fromUid = intent.getIntExtra("from.uid", -1);
        if (fromUid < 0) return;
        if (fromUid == Process.myUid()) return;

        App app = App.self;
        PackageManager pm = app.getPackageManager();
        Policy policy;

        boolean notify;
        Bundle data = intent.getExtras();
        if (data.containsKey("notify")) {
            notify = data.getBoolean("notify");
            try {
                policy = new Policy(fromUid, pm);
            } catch (PackageManager.NameNotFoundException e) {
                return;
            }
        } else {
            // Doesn't report whether notify or not, check database ourselves
            policy = app.mDB.getPolicy(fromUid);
            if (policy == null)
                return;
            notify = policy.notification;
        }

        policy.policy = data.getInt("policy", -1);
        if (policy.policy < 0)
            return;

        if (notify)
            handleNotify(policy);

        SuLogEntry log = new SuLogEntry(policy);

        int toUid = intent.getIntExtra("to.uid", -1);
        if (toUid < 0) return;
        int pid = intent.getIntExtra("pid", -1);
        if (pid < 0) return;
        String command = intent.getStringExtra("command");
        if (command == null) return;
        log.toUid = toUid;
        log.fromPid = pid;
        log.command = command;
        log.date = new Date();
        app.mDB.addLog(log);
    }

    private void handleNotify(Policy policy) {
        if (policy.notification &&
                (int) Config.get(Config.Key.SU_NOTIFICATION) == Config.Value.NOTIFICATION_TOAST)
            Utils.toast(getMessage(policy), Toast.LENGTH_SHORT);
    }

    public void handleNotify(Intent intent) {
        int fromUid = intent.getIntExtra("from.uid", -1);
        if (fromUid < 0) return;
        if (fromUid == Process.myUid()) return;
        try {
            Policy policy = new Policy(fromUid, App.self.getPackageManager());
            policy.policy = intent.getIntExtra("policy", -1);
            if (policy.policy >= 0)
                handleNotify(policy);
        } catch (PackageManager.NameNotFoundException ignored) {}
    }

    public abstract String getMessage(Policy policy);
}
