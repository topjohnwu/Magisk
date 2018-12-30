package com.topjohnwu.core.database;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import com.topjohnwu.core.App;
import com.topjohnwu.core.Const;
import com.topjohnwu.core.Data;
import com.topjohnwu.core.container.Repo;

import java.util.HashSet;
import java.util.Set;

public class RepoDatabaseHelper extends SQLiteOpenHelper {

    private static final int DATABASE_VER = 4;
    private static final String TABLE_NAME = "repos";

    private SQLiteDatabase mDb;
    private Runnable adapterCb;

    public RepoDatabaseHelper(Context context) {
        super(context, "repo.db", null, DATABASE_VER);
        mDb = getWritableDatabase();

        // Remove outdated repos
        mDb.delete(TABLE_NAME, "minMagisk<?", new String[] { String.valueOf(Const.MIN_MODULE_VER) });
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        onUpgrade(db, 0, DATABASE_VER);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        if (oldVersion != newVersion) {
            // Nuke old DB and create new table
            db.execSQL("DROP TABLE IF EXISTS " + TABLE_NAME);
            db.execSQL(
                    "CREATE TABLE IF NOT EXISTS " + TABLE_NAME + " " +
                            "(id TEXT, name TEXT, version TEXT, versionCode INT, minMagisk INT, " +
                            "author TEXT, description TEXT, last_update INT, PRIMARY KEY(id))");
            App.self.prefs.edit().remove(Const.Key.ETAG_KEY).apply();
        }
    }

    @Override
    public void onDowngrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        onUpgrade(db, 0, DATABASE_VER);
    }

    public void clearRepo() {
        mDb.delete(TABLE_NAME, null, null);
        notifyAdapter();
    }


    public void removeRepo(String id) {
        mDb.delete(TABLE_NAME, "id=?", new String[] { id });
        notifyAdapter();
    }

    public void removeRepo(Repo repo) {
        removeRepo(repo.getId());
    }

    public void removeRepo(Iterable<String> list) {
        for (String id : list) {
            if (id == null) continue;
            mDb.delete(TABLE_NAME, "id=?", new String[] { id });
        }
        notifyAdapter();
    }

    public void addRepo(Repo repo) {
        mDb.replace(TABLE_NAME, null, repo.getContentValues());
        notifyAdapter();
    }

    public Repo getRepo(String id) {
        try (Cursor c = mDb.query(TABLE_NAME, null, "id=?", new String[] { id }, null, null, null)) {
            if (c.moveToNext()) {
                return new Repo(c);
            }
        }
        return null;
    }

    public Cursor getRawCursor() {
        return mDb.query(TABLE_NAME, null, null, null, null, null, null);
    }

    public Cursor getRepoCursor() {
        String orderBy = null;
        switch (Data.repoOrder) {
            case Const.Value.ORDER_NAME:
                orderBy = "name COLLATE NOCASE";
                break;
            case Const.Value.ORDER_DATE:
                orderBy = "last_update DESC";
        }
        return mDb.query(TABLE_NAME, null, "minMagisk<=? AND minMagisk>=?",
                new String[] { String.valueOf(Data.magiskVersionCode), String.valueOf(Const.MIN_MODULE_VER) },
                null, null, orderBy);
    }

    public Set<String> getRepoIDSet() {
        HashSet<String> set = new HashSet<>(300);
        try (Cursor c = mDb.query(TABLE_NAME, null, null, null, null, null, null)) {
            while (c.moveToNext()) {
                set.add(c.getString(c.getColumnIndex("id")));
            }
        }
        return set;
    }

    public void registerAdapterCallback(Runnable cb) {
        adapterCb = cb;
    }

    public void unregisterAdapterCallback() {
        adapterCb = null;
    }

    private void notifyAdapter() {
        if (adapterCb != null) {
            App.mainHandler.post(adapterCb);
        }
    }
}
