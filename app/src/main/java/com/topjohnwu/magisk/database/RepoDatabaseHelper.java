package com.topjohnwu.magisk.database;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;

import java.util.LinkedList;
import java.util.List;

public class RepoDatabaseHelper extends SQLiteOpenHelper {

    private static final int DATABASE_VER = 2;
    private static final String TABLE_NAME = "repos";
    private static final int MIN_TEMPLATE_VER = 3;

    private SQLiteDatabase mDb;
    private MagiskManager mm;

    public RepoDatabaseHelper(Context context) {
        super(context, "repo.db", null, DATABASE_VER);
        mDb = getWritableDatabase();
        mm = Utils.getMagiskManager(context);
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
                    "(id TEXT, name TEXT, version TEXT, versionCode INT, " +
                    "author TEXT, description TEXT, repo_name TEXT, last_update INT, " +
                    "PRIMARY KEY(id))");
            oldVersion++;
        }
        if (oldVersion == 1) {
            db.execSQL("ALTER TABLE " + TABLE_NAME + " ADD template INT");
            oldVersion++;
        }
    }

    public void clearRepo() {
        mDb.delete(TABLE_NAME, null, null);
    }

    public void removeRepo(List<String> list) {
        for (String id : list) {
            Logger.dev("Remove from DB: " + id);
            mDb.delete(TABLE_NAME, "id=?", new String[] { id });
        }
    }

    public void addRepo(Repo repo) {
        mDb.replace(TABLE_NAME, null, repo.getContentValues());
    }

    public Repo getRepo(String id) {
        try (Cursor c = mDb.query(TABLE_NAME, null, "id=?", new String[] { id }, null, null, null)) {
            if (c.moveToNext()) {
                return new Repo(c);
            }
        }
        return null;
    }

    public Cursor getRepoCursor() {
        return mDb.query(TABLE_NAME, null, "template>=? AND template<=?", new String[] { String.valueOf(MIN_TEMPLATE_VER), String.valueOf(mm.magiskVersionCode) }, null, null, "name COLLATE NOCASE");
    }

    public List<String> getRepoIDList() {
        LinkedList<String> ret = new LinkedList<>();
        try (Cursor c = mDb.query(TABLE_NAME, null, null, null, null, null, null)) {
            while (c.moveToNext()) {
                ret.add(c.getString(c.getColumnIndex("id")));
            }
        }
        return ret;
    }
}
