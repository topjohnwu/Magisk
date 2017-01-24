package com.topjohnwu.magisk.superuser;

import android.content.ContentValues;
import android.database.Cursor;


public class Policy {
    public int uid, policy;
    public long until;
    public boolean logging, notification;
    public String packageName, appName;

    public Policy() {}

    public Policy(Cursor c) {
        uid = c.getInt(c.getColumnIndex("uid"));
        packageName = c.getString(c.getColumnIndex("package_name"));
        appName = c.getString(c.getColumnIndex("app_name"));
        policy = c.getInt(c.getColumnIndex("policy"));
        until = c.getLong(c.getColumnIndex("until"));
        logging = c.getInt(c.getColumnIndex("logging")) != 0;
        notification = c.getInt(c.getColumnIndex("notification")) != 0;
    }
    
    public ContentValues getContentValues() {
        ContentValues values = new ContentValues();
        values.put("uid", uid);
        values.put("package_name", packageName);
        values.put("app_name", appName);
        values.put("policy",policy);
        values.put("until", until);
        values.put("logging", logging ? 1 : 0);
        values.put("notification", notification ? 1 : 0);
        return values;
    }
}
