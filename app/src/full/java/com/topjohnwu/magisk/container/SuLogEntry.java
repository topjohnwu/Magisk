package com.topjohnwu.magisk.container;

import android.content.ContentValues;

import com.topjohnwu.magisk.utils.LocaleManager;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

public class SuLogEntry {

    public int fromUid, toUid, fromPid;
    public String packageName, appName, command;
    public boolean action;
    public Date date;

    public SuLogEntry(Policy policy) {
        fromUid = policy.uid;
        packageName = policy.packageName;
        appName = policy.appName;
        action = policy.policy == Policy.ALLOW;
    }

    public SuLogEntry(ContentValues values) {
        fromUid = values.getAsInteger("from_uid");
        packageName = values.getAsString("package_name");
        appName = values.getAsString("app_name");
        fromPid = values.getAsInteger("from_pid");
        command = values.getAsString("command");
        toUid = values.getAsInteger("to_uid");
        action = values.getAsInteger("action") != 0;
        date = new Date(values.getAsLong("time"));
    }

    public ContentValues getContentValues() {
        ContentValues values = new ContentValues();
        values.put("from_uid", fromUid);
        values.put("package_name", packageName);
        values.put("app_name", appName);
        values.put("from_pid", fromPid);
        values.put("command", command);
        values.put("to_uid", toUid);
        values.put("action", action ? 1 : 0);
        values.put("time", date.getTime());
        return values;
    }

    public String getDateString() {
        return DateFormat.getDateInstance(DateFormat.MEDIUM, LocaleManager.locale).format(date);
    }

    public String getTimeString() {
        return new SimpleDateFormat("h:mm a", LocaleManager.locale).format(date);
    }
}
