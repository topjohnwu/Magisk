package com.topjohnwu.magisk.superuser;

import android.content.ContentValues;
import android.database.Cursor;

public class SuLogEntry {

    public int fromUid, toUid, fromPid;
    public String packageName, appName, command;
    public boolean action;
    public long time;

    public SuLogEntry(Policy policy) {
        fromUid = policy.uid;
        packageName = policy.packageName;
        appName = policy.appName;
    }

    public SuLogEntry(Cursor c) {
        fromUid = c.getInt(c.getColumnIndex("from_uid"));
        fromPid = c.getInt(c.getColumnIndex("from_pid"));
        toUid = c.getInt(c.getColumnIndex("to_uid"));
        packageName = c.getString(c.getColumnIndex("package_name"));
        appName = c.getString(c.getColumnIndex("app_name"));
        command = c.getString(c.getColumnIndex("command"));
        action = c.getInt(c.getColumnIndex("action")) != 0;
        time = c.getLong(c.getColumnIndex("until"));
    }

    public ContentValues getContentValues() {
        ContentValues values = new ContentValues();
        values.put("from_uid", fromUid);
        values.put("package_name", packageName);
        values.put("app_name", appName);
        values.put("from_pid", fromPid);
        values.put("command", command);
        values.put("to_uid", toUid);
        values.put("action", action);
        values.put("time", time);
        return values;
    }
}
