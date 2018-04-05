package com.topjohnwu.magisk.superuser;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Process;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.container.Policy;
import com.topjohnwu.magisk.container.SuLogEntry;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Utils;

import java.util.Date;

public class SuReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        int fromUid, toUid, pid, mode;
        String command, action;
        Policy policy;

        MagiskManager mm = Utils.getMagiskManager(context);

        if (intent == null) return;

        mode = intent.getIntExtra("mode", -1);
        if (mode < 0) return;

        if (mode == Const.Value.NOTIFY_USER_TO_OWNER) {
            MagiskManager.toast(R.string.multiuser_hint_owner_request, Toast.LENGTH_LONG);
            return;
        }

        fromUid = intent.getIntExtra("from.uid", -1);
        if (fromUid < 0) return;
        if (fromUid == Process.myUid()) return; // Don't show anything if it's Magisk Manager

        action = intent.getStringExtra("action");
        if (action == null) return;

        policy = mm.mDB.getPolicy(fromUid);
        if (policy == null) {
            try {
                policy = new Policy(fromUid, context.getPackageManager());
            } catch (PackageManager.NameNotFoundException e) {
                e.printStackTrace();
                return;
            }
        }

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

        if (policy.notification && mm.suNotificationType == Const.Value.NOTIFICATION_TOAST) {
            MagiskManager.toast(message, Toast.LENGTH_SHORT);
        }

        if (mode == Const.Value.NOTIFY_NORMAL_LOG && policy.logging) {
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
            mm.mDB.addLog(log);
        }
    }
}
