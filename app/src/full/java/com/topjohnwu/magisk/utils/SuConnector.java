package com.topjohnwu.magisk.utils;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
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

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.Date;

public abstract class SuConnector {

    private LocalSocket socket;
    protected DataOutputStream out;
    protected DataInputStream in;

    public SuConnector(String name) throws IOException {
        socket = new LocalSocket();
        socket.connect(new LocalSocketAddress(name, LocalSocketAddress.Namespace.ABSTRACT));
        out = new DataOutputStream(new BufferedOutputStream(socket.getOutputStream()));
        in = new DataInputStream(new BufferedInputStream(socket.getInputStream()));
    }

    private String readString() throws IOException {
        int len = in.readInt();
        byte[] buf = new byte[len];
        in.readFully(buf);
        return new String(buf, "UTF-8");
    }

    public Bundle readSocketInput() throws IOException {
        Bundle bundle = new Bundle();
        while (true) {
            String name = readString();
            if (TextUtils.equals(name, "eof"))
                break;
            bundle.putString(name, readString());
        }
        return bundle;
    }

    public void response() {
        try {
            onResponse();
            out.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            in.close();
            out.close();
            socket.close();
        } catch (IOException ignored) { }
    }

    protected abstract void onResponse() throws IOException;

    public static void handleLogs(Intent intent) {

        int fromUid = intent.getIntExtra("from.uid", -1);
        if (fromUid < 0) return;
        if (fromUid == Process.myUid()) return;

        MagiskManager mm = Data.MM();
        PackageManager pm = mm.getPackageManager();
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
            policy = mm.mDB.getPolicy(fromUid);
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
        mm.mDB.addLog(log);
    }

    private static void handleNotify(Policy policy) {
        MagiskManager mm = Data.MM();
        String message = mm.getString(policy.policy == Policy.ALLOW ?
                R.string.su_allow_toast : R.string.su_deny_toast, policy.appName);
        if (policy.notification && Data.suNotificationType == Const.Value.NOTIFICATION_TOAST)
            Utils.toast(message, Toast.LENGTH_SHORT);
    }

    public static void handleNotify(Intent intent) {
        MagiskManager mm = Data.MM();
        int fromUid = intent.getIntExtra("from.uid", -1);
        if (fromUid < 0) return;
        if (fromUid == Process.myUid()) return;
        try {
            Policy policy = new Policy(fromUid, mm.getPackageManager());
            policy.policy = intent.getIntExtra("policy", -1);
            if (policy.policy >= 0)
                handleNotify(policy);
        } catch (PackageManager.NameNotFoundException ignored) {}
    }
}
