package com.topjohnwu.magisk.superuser;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Process;
import android.widget.Toast;

import com.topjohnwu.magisk.R;

public class SuReceiver extends BroadcastReceiver {
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

        if (policy.notification) {
            String message;
            switch (action) {
                case "allow":
                    message = context.getString(R.string.su_allow_toast, policy.appName);
                    break;
                case "deny":
                    message = context.getString(R.string.su_deny_toast, policy.appName);
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
            SuLogEntry log = new SuLogEntry(policy);
            log.toUid = toUid;
            log.fromPid = pid;
            log.command = command;
            SuLogDatabaseHelper logDbHelper = new SuLogDatabaseHelper(context);
            logDbHelper.addLog(log);
        }
    }
}
