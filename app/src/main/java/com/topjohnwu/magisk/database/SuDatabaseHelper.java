package com.topjohnwu.magisk.database;

import android.content.Context;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import com.topjohnwu.magisk.superuser.Policy;

import java.util.ArrayList;
import java.util.List;

public class SuDatabaseHelper extends SQLiteOpenHelper {

    private static final int DATABASE_VER = 1;
    private static final String TABLE_NAME = "policies";

    public SuDatabaseHelper(Context context) {
        super(context, "su.db", null, DATABASE_VER);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        onUpgrade(db, 0, DATABASE_VER);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        if (oldVersion == 0) {
            db.execSQL(
                    "CREATE TABLE IF NOT EXISTS " + TABLE_NAME + " " +
                    "(uid INT, package_name TEXT, app_name TEXT, policy INT, " +
                    "until INT, logging INT, notification INT, " +
                    "PRIMARY KEY(uid))");
        }
        // Currently new database, no upgrading
    }

    public boolean deletePolicy(Policy policy) {
        SQLiteDatabase db = getWritableDatabase();
        db.delete(TABLE_NAME, "package_name=?", new String[] { policy.packageName });
        db.close();
        return getPolicy(policy.packageName) == null;
    }

    public Policy getPolicy(int uid) {
        Policy policy = null;
        SQLiteDatabase db = getReadableDatabase();
        try (Cursor c = db.query(TABLE_NAME, null, "uid=?", new String[] { String.valueOf(uid) }, null, null, null)) {
            if (c.moveToNext()) {
                policy = new Policy(c);
            }
        }
        db.close();
        return policy;
    }

    public Policy getPolicy(String pkg) {
        Policy policy = null;
        SQLiteDatabase db = getReadableDatabase();
        try (Cursor c = db.query(TABLE_NAME, null, "package_name=?", new String[] { pkg }, null, null, null)) {
            if (c.moveToNext()) {
                policy = new Policy(c);
            }
        }
        db.close();
        return policy;
    }

    public void addPolicy(Policy policy) {
        SQLiteDatabase db = getWritableDatabase();
        db.replace(TABLE_NAME, null, policy.getContentValues());
        db.close();
    }

    public void updatePolicy(Policy policy) {
        SQLiteDatabase db = getWritableDatabase();
        db.update(TABLE_NAME, policy.getContentValues(), "package_name=?",
                new String[] { policy.packageName });
        db.close();
    }

    public List<Policy> getPolicyList(PackageManager pm) {
        List<Policy> ret = new ArrayList<>();
        SQLiteDatabase db = getWritableDatabase();
        Policy policy;
        // Clear outdated policies
        db.delete(TABLE_NAME, "until > 0 AND until < ?",
                new String[] { String.valueOf(System.currentTimeMillis()) });
        try (Cursor c = db.query(TABLE_NAME, null, null, null, null, null, "app_name ASC")) {
            while (c.moveToNext()) {
                policy = new Policy(c);
                ret.add(policy);
            }
        }
        db.close();
        return ret;
    }
}
