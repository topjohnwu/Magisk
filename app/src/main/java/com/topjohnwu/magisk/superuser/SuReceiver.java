package com.topjohnwu.magisk.superuser;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Process;
import android.widget.Toast;

import com.topjohnwu.magisk.Global;
import com.topjohnwu.magisk.R;

import java.util.Date;

public class SuReceiver extends BroadcastReceiver {

    private static final int NO_NOTIFICATION = 0;
    private static final int TOAST = 1;


    @Override
    public void onReceive(Context context, Intent intent) {
        int fromUid, toUid, pid;
        String command, action;
        Policy policy;

        if (intent == null) return;

        fromUid = intent.getIntExtra("from.uid", -1);
        if (fromUid < 0) return;
        if (fromUid == Process.myUid()) return; // Don't show anything if it's Magisk Manager

        action = intent.getStringExtra("action");
        if (action == null) return;

        SuDatabaseHelper suDbHelper = new SuDatabaseHelper(context);
        policy = suDbHelper.getPolicy(fromUid);
        if (policy == null) try {
            policy = new Policy(fromUid, context.getPackageManager());
        } catch (Throwable throwable) {
            return;
        }

        Global.initSuConfigs(context);

        SuLogEntry log = new SuLogEntry(policy);

        if (policy.notification && Global.Configs.suNotificationType == TOAST) {
            String message;
            switch (action) {
                case "allow":
                    message = context.getString(R.string.su_allow_toast, policy.appName);
                    log.action = true;
                    break;
                case "deny":
                    message = context.getString(R.string.su_deny_toast, policy.appName);
                    log.action = false;
                    break;
                default:
                    return;
            }
            Toast.makeText(context, message, Toast.LENGTH_SHORT).show();
        }
        if (policy.logging) {
            toUid = intent.getIntExtra("to.uid", -1);
            if (toUid < 0) return;
            pid = intent.getIntExtra("pid", -1);
            if (pid < 0) return;
            command = intent.getStringExtra("command");
            if (command == null) return;
            log.toUid = toUid;
            log.fromPid = pid;
            log.command = command;
            log.date = new Date();
            SuLogDatabaseHelper logDbHelper = new SuLogDatabaseHelper(context);
            logDbHelper.addLog(log);
        }
    }
}
