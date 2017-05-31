package com.topjohnwu.magisk.superuser;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Process;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;

import java.util.Date;

public class SuReceiver extends BroadcastReceiver {

    private static final int NO_NOTIFICATION = 0;
    private static final int TOAST = 1;
    private static final int NOTIFY_NORMAL_LOG = 0;
    private static final int NOTIFY_USER_TOASTS = 1;
    private static final int NOTIFY_USER_TO_OWNER = 2;

    @Override
    public void onReceive(Context context, Intent intent) {
        int fromUid, toUid, pid, mode;
        String command, action;
        Policy policy;

        MagiskManager magiskManager = (MagiskManager) context.getApplicationContext();

        if (intent == null) return;

        mode = intent.getIntExtra("mode", -1);
        if (mode < 0) return;

        if (mode == NOTIFY_USER_TO_OWNER) {
            magiskManager.toast(R.string.multiuser_hint_owner_request, Toast.LENGTH_LONG);
            return;
        }

        fromUid = intent.getIntExtra("from.uid", -1);
        if (fromUid < 0) return;
        if (fromUid == Process.myUid()) return; // Don't show anything if it's Magisk Manager

        action = intent.getStringExtra("action");
        if (action == null) return;

        policy = magiskManager.suDB.getPolicy(fromUid);
        if (policy == null) {
            try {
                policy = new Policy(fromUid, context.getPackageManager());
            } catch (PackageManager.NameNotFoundException e) {
                e.printStackTrace();
                return;
            }
        }

        magiskManager.initSUConfig();

        SuLogEntry log = new SuLogEntry(policy);

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

        if (policy.notification && magiskManager.suNotificationType == TOAST) {
            magiskManager.toast(message, Toast.LENGTH_SHORT);
        }

        if (mode == NOTIFY_NORMAL_LOG && policy.logging) {
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
            magiskManager.suDB.addLog(log);
        }
    }
}
