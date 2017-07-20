package com.topjohnwu.magisk.database;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.ValueSortedMap;

import java.util.Collection;

public class RepoDatabaseHelper extends SQLiteOpenHelper {

    private static final int DATABASE_VER = 2;
    private static final String TABLE_NAME = "repos";
    private static final int MIN_TEMPLATE_VER = 3;

    private SQLiteDatabase mDb;

    public RepoDatabaseHelper(Context context) {
        super(context, "repo.db", null, DATABASE_VER);
        mDb = getWritableDatabase();
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

    public void addRepoMap(ValueSortedMap<String, Repo> map) {
        Collection<Repo> list = map.values();
        for (Repo repo : list) {
            Logger.dev("Add to DB: " + repo.getId());
            mDb.replace(TABLE_NAME, null, repo.getContentValues());
        }
    }

    public void clearRepo() {
        mDb.delete(TABLE_NAME, null, null);
    }

    public void removeRepo(ValueSortedMap<String, Repo> map) {
        Collection<Repo> list = map.values();
        for (Repo repo : list) {
            Logger.dev("Remove from DB: " + repo.getId());
            mDb.delete(TABLE_NAME, "id=?", new String[] { repo.getId() });
        }
    }

    public ValueSortedMap<String, Repo> getRepoMap() {
        return getRepoMap(true);
    }

    public ValueSortedMap<String, Repo> getRepoMap(boolean filtered) {
        ValueSortedMap<String, Repo> ret = new ValueSortedMap<>();
        Repo repo;
        try (Cursor c = mDb.query(TABLE_NAME, null, "template>=?", new String[] { filtered ? String.valueOf(MIN_TEMPLATE_VER) : "0" }, null, null, null)) {
            while (c.moveToNext()) {
                repo = new Repo(c);
                ret.put(repo.getId(), repo);
            }
        }
        return ret;
    }
}
