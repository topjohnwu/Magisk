package com.topjohnwu.magisk.database;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.superuser.SuLogEntry;

import java.util.ArrayList;
import java.util.List;

public class SuLogDatabaseHelper extends SQLiteOpenHelper {

    private static final int DATABASE_VER = 1;
    private static final String TABLE_NAME = "logs";

    private MagiskManager magiskManager;

    public SuLogDatabaseHelper(Context context) {
        super(context, "sulog.db", null, DATABASE_VER);
        magiskManager = (MagiskManager) context.getApplicationContext();
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL(
                "CREATE TABLE IF NOT EXISTS " + TABLE_NAME + " (id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                "from_uid INT, package_name TEXT, app_name TEXT, from_pid INT, " +
                "to_uid INT, action INT, time INT, command TEXT)");
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        // Currently new database, no upgrading
    }

    public void addLog(SuLogEntry log) {
        SQLiteDatabase db = getWritableDatabase();
        db.insert(TABLE_NAME, null, log.getContentValues());
        db.close();
    }

    public void clearLogs() {
        SQLiteDatabase db = getWritableDatabase();
        db.delete(TABLE_NAME, null, null);
        db.close();
    }

    public List<SuLogEntry> getLogList() {
        return getLogList(null);
    }

    public List<SuLogEntry> getLogList(int uid) {
        return getLogList("uid=" + uid);
    }

    public List<SuLogEntry> getLogList(String selection) {
        List<SuLogEntry> ret = new ArrayList<>();
        SQLiteDatabase db = getWritableDatabase();
        // Clear outdated logs
        db.delete(TABLE_NAME, "time < ?", new String[] { String.valueOf(
                System.currentTimeMillis() / 1000 - magiskManager.suLogTimeout * 86400) });
        try (Cursor c = db.query(TABLE_NAME, null, selection, null, null, null, "time DESC")) {
            while (c.moveToNext())
                ret.add(new SuLogEntry(c));
        }
        db.close();
        return ret;
    }
}
