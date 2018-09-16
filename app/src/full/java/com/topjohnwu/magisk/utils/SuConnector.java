package com.topjohnwu.magisk.utils;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.LocalSocket;
import android.os.Bundle;
import android.os.Process;
import android.text.TextUtils;
import android.widget.Toast;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.container.Policy;
import com.topjohnwu.magisk.container.SuLogEntry;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.Date;

public abstract class SuConnector {

    protected LocalSocket socket = new LocalSocket();

    private String readString(DataInputStream is) throws IOException {
        int len = is.readInt();
        byte[] buf = new byte[len];
        is.readFully(buf);
        return new String(buf);
    }

    public Bundle readSocketInput() throws IOException {
        Bundle bundle = new Bundle();
        DataInputStream is = new DataInputStream(socket.getInputStream());
        while (true) {
            String name = readString(is);
            if (TextUtils.equals(name, "eof"))
                break;
            bundle.putString(name, readString(is));
        }
        return bundle;
    }

    protected DataOutputStream getOutputStream() throws IOException {
        return new DataOutputStream(socket.getOutputStream());
    }

    public abstract void response();

    public static void handleLogs(Intent intent, int version) {
        MagiskManager mm = Data.MM();

        if (intent == null) return;

        int fromUid = intent.getIntExtra("from.uid", -1);
        if (fromUid < 0) return;
        if (fromUid == Process.myUid()) return;

        Policy policy = mm.mDB.getPolicy(fromUid);
        if (policy == null) {
            try {
                policy = new Policy(fromUid, mm.getPackageManager());
            } catch (PackageManager.NameNotFoundException e) {
                e.printStackTrace();
                return;
            }
        }

        SuLogEntry log = new SuLogEntry(policy);
        if (version == 1) {
            String action = intent.getStringExtra("action");
            if (action == null) return;
            switch (action) {
                case "allow":
                    log.action = true;
                    break;
                case "deny":
                    log.action = false;
                    break;
                default:
                    return;
            }
        } else {
            switch (intent.getIntExtra("policy", -1)) {
                case Policy.ALLOW:
                    log.action = true;
                    break;
                case Policy.DENY:
                    log.action = false;
                    break;
                default:
                    return;
            }
        }

        String message = mm.getString(log.action ?
                R.string.su_allow_toast : R.string.su_deny_toast, policy.appName);

        if (policy.notification && Data.suNotificationType == Const.Value.NOTIFICATION_TOAST)
            Utils.toast(message, Toast.LENGTH_SHORT);

        if (policy.logging) {
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
            mm.mDB.addLog(log);
        }
    }
}
