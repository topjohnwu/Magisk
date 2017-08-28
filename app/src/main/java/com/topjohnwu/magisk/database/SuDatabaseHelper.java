package com.topjohnwu.magisk.database;

import android.content.ContentValues;
import android.content.Context;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.database.DatabaseUtils;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.text.TextUtils;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.superuser.Policy;
import com.topjohnwu.magisk.superuser.SuLogEntry;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;

public class SuDatabaseHelper extends SQLiteOpenHelper {

    public static final String ROOT_ACCESS = "root_access";
    public static final int ROOT_ACCESS_DISABLED = 0;
    public static final int ROOT_ACCESS_APPS_ONLY = 1;
    public static final int ROOT_ACCESS_ADB_ONLY = 2;
    public static final int ROOT_ACCESS_APPS_AND_ADB = 3;

    public static final String MULTIUSER_MODE = "multiuser_mode";
    public static final int MULTIUSER_MODE_OWNER_ONLY = 0;
    public static final int MULTIUSER_MODE_OWNER_MANAGED = 1;
    public static final int MULTIUSER_MODE_USER = 2;

    public static final String MNT_NS = "mnt_ns";
    public static final int NAMESPACE_MODE_GLOBAL = 0;
    public static final int NAMESPACE_MODE_REQUESTER = 1;
    public static final int NAMESPACE_MODE_ISOLATE = 2;

    private static final int DATABASE_VER = 3;
    private static final String POLICY_TABLE = "policies";
    private static final String LOG_TABLE = "logs";
    private static final String SETTINGS_TABLE = "settings";

    private MagiskManager magiskManager;
    private PackageManager pm;
    private SQLiteDatabase mDb;

    public SuDatabaseHelper(Context context) {
        super(context, "su.db", null, DATABASE_VER);
        magiskManager = Utils.getMagiskManager(context);
        pm = context.getPackageManager();
        mDb = getWritableDatabase();
        cleanup();
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        onUpgrade(db, 0, DATABASE_VER);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        if (oldVersion == 0) {
            createTables(db);
            oldVersion = 2;
        }
        if (oldVersion == 1) {
            // We're dropping column app_name, rename and re-construct table
            db.execSQL("ALTER TABLE " + POLICY_TABLE + " RENAME TO " + POLICY_TABLE + "_old");

            // Create the new tables
            createTables(db);

            // Migrate old data to new tables
            db.execSQL(
                    "INSERT INTO " + POLICY_TABLE + " SELECT " +
                    "uid, package_name, policy, until, logging, notification " +
                    "FROM " + POLICY_TABLE + "_old");
            db.execSQL("DROP TABLE " + POLICY_TABLE + "_old");

            File oldDB = magiskManager.getDatabasePath("sulog.db");
            if (oldDB.exists()) {
                migrateLegacyLogList(oldDB, db);
                magiskManager.deleteDatabase("sulog.db");
            }
            ++oldVersion;
        }
        if (oldVersion == 2) {
            db.execSQL("UPDATE " + LOG_TABLE + " SET time=time*1000");
            ++oldVersion;
        }
    }

    private void createTables(SQLiteDatabase db) {
        // Policies
        db.execSQL(
                "CREATE TABLE IF NOT EXISTS " + POLICY_TABLE + " " +
                "(uid INT, package_name TEXT, policy INT, " +
                "until INT, logging INT, notification INT, " +
                "PRIMARY KEY(uid))");

        // Logs
        db.execSQL(
                "CREATE TABLE IF NOT EXISTS " + LOG_TABLE + " " +
                "(from_uid INT, package_name TEXT, app_name TEXT, from_pid INT, " +
                "to_uid INT, action INT, time INT, command TEXT)");

        // Settings
        db.execSQL(
                "CREATE TABLE IF NOT EXISTS " + SETTINGS_TABLE + " " +
                "(key TEXT, value INT, PRIMARY KEY(key))");
    }

    private void cleanup() {
        // Clear outdated policies
        mDb.delete(POLICY_TABLE, "until > 0 AND until < ?",
                new String[] { String.valueOf(System.currentTimeMillis() / 1000) });
        // Clear outdated logs
        mDb.delete(LOG_TABLE, "time < ?", new String[] { String.valueOf(
                System.currentTimeMillis() - magiskManager.suLogTimeout * 86400000) });
    }

    public void deletePolicy(Policy policy) {
        deletePolicy(policy.packageName);
    }

    public void deletePolicy(String pkg) {
        mDb.delete(POLICY_TABLE, "package_name=?", new String[] { pkg });
    }

    public void deletePolicy(int uid) {
        mDb.delete(POLICY_TABLE, "uid=?", new String[]{String.valueOf(uid)});
    }

    public Policy getPolicy(int uid) {
        Policy policy = null;
        try (Cursor c = mDb.query(POLICY_TABLE, null, "uid=?", new String[] { String.valueOf(uid) }, null, null, null)) {
            if (c.moveToNext()) {
                policy = new Policy(c, pm);
            }
        } catch (PackageManager.NameNotFoundException e) {
            deletePolicy(uid);
            return null;
        }
        return policy;
    }

    public Policy getPolicy(String pkg) {
        Policy policy = null;
        try (Cursor c = mDb.query(POLICY_TABLE, null, "package_name=?", new String[] { pkg }, null, null, null)) {
            if (c.moveToNext()) {
                policy = new Policy(c, pm);
            }
        } catch (PackageManager.NameNotFoundException e) {
            deletePolicy(pkg);
            return null;
        }
        return policy;
    }

    public void addPolicy(Policy policy) {
        mDb.replace(POLICY_TABLE, null, policy.getContentValues());
    }

    public void updatePolicy(Policy policy) {
        mDb.update(POLICY_TABLE, policy.getContentValues(), "package_name=?",
                new String[] { policy.packageName });
    }

    public List<Policy> getPolicyList(PackageManager pm) {
        try (Cursor c = mDb.query(POLICY_TABLE, null, null, null, null, null, null)) {
            List<Policy> ret = new ArrayList<>(c.getCount());
            while (c.moveToNext()) {
                try {
                    Policy policy = new Policy(c, pm);
                    // The application changed UID for some reason, check user config
                    if (policy.info.uid != policy.uid) {
                        if (magiskManager.suReauth) {
                            // Reauth required, remove from DB
                            deletePolicy(policy);
                            continue;
                        } else {
                            // No reauth, update to use the new UID
                            policy.uid = policy.info.uid;
                            updatePolicy(policy);
                        }
                    }
                    ret.add(policy);
                } catch (PackageManager.NameNotFoundException e) {
                    // The app no longer exist, remove from DB
                    deletePolicy(c.getInt(c.getColumnIndex("uid")));
                }
            }
            Collections.sort(ret);
            return ret;
        }
    }

    public List<List<Integer>> getLogStructure() {
        try (Cursor c = mDb.query(LOG_TABLE, new String[] { "time" }, null, null, null, null, "time DESC")) {
            List<List<Integer>> ret = new ArrayList<>();
            List<Integer> list = null;
            String dateString = null, newString;
            while (c.moveToNext()) {
                Date date = new Date(c.getLong(c.getColumnIndex("time")));
                newString = DateFormat.getDateInstance(DateFormat.MEDIUM, MagiskManager.locale).format(date);
                if (!TextUtils.equals(dateString, newString)) {
                    dateString = newString;
                    list = new ArrayList<>();
                    ret.add(list);
                }
                list.add(c.getPosition());
            }
            return ret;
        }
    }

    public Cursor getLogCursor() {
        return getLogCursor(mDb);
    }

    public Cursor getLogCursor(SQLiteDatabase db) {
        return db.query(LOG_TABLE, null, null, null, null, null, "time DESC");
    }

    private void migrateLegacyLogList(File oldDB, SQLiteDatabase newDB) {
        try (SQLiteDatabase oldDb = SQLiteDatabase.openDatabase(oldDB.getPath(), null, SQLiteDatabase.OPEN_READWRITE);
             Cursor c = getLogCursor(oldDb)) {
            while (c.moveToNext()) {
                ContentValues values = new ContentValues();
                DatabaseUtils.cursorRowToContentValues(c, values);
                newDB.insert(LOG_TABLE, null, values);
            }
        }
    }

    public void addLog(SuLogEntry log) {
        mDb.insert(LOG_TABLE, null, log.getContentValues());
    }

    public void clearLogs() {
        mDb.delete(LOG_TABLE, null, null);
    }

    public void setSettings(String key, int value) {
        ContentValues data = new ContentValues();
        data.put("key", key);
        data.put("value", value);
        mDb.replace(SETTINGS_TABLE, null, data);
    }

    public int getSettings(String key, int defaultValue) {
        int value = defaultValue;
        try (Cursor c = mDb.query(SETTINGS_TABLE, null, "key=?",new String[] { key }, null, null, null)) {
            if (c.moveToNext()) {
                value = c.getInt(c.getColumnIndex("value"));
            }
        }
        return value;
    }
}
