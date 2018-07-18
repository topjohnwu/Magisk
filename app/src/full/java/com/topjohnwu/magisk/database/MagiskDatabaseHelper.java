package com.topjohnwu.magisk.database;

import android.content.ContentValues;
import android.content.Context;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.os.Build;
import android.os.Process;
import android.support.annotation.NonNull;
import android.text.TextUtils;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.container.Policy;
import com.topjohnwu.magisk.container.SuLogEntry;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.io.SuFile;

import java.io.File;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;

public class MagiskDatabaseHelper {

    private static final int DATABASE_VER = 5;
    private static final String POLICY_TABLE = "policies";
    private static final String LOG_TABLE = "logs";
    private static final String SETTINGS_TABLE = "settings";
    private static final String STRINGS_TABLE = "strings";

    private PackageManager pm;
    private SQLiteDatabase db;

    @NonNull
    public static MagiskDatabaseHelper getInstance(MagiskManager mm) {
        try {
            return new MagiskDatabaseHelper(mm);
        } catch (Exception e) {
            // Let's cleanup everything and try again
            Shell.Sync.su("db_clean '*'");
            return new MagiskDatabaseHelper(mm);
        }
    }

    private MagiskDatabaseHelper(MagiskManager mm) {
        pm = mm.getPackageManager();
        db = openDatabase(mm);
        db.disableWriteAheadLogging();
        int version = db.getVersion();
        if (version < DATABASE_VER) {
            onUpgrade(db, version);
        } else if (version > DATABASE_VER) {
            onDowngrade(db);
        }
        db.setVersion(DATABASE_VER);
        clearOutdated();
    }

    private SQLiteDatabase openDatabase(MagiskManager mm) {
        final File DB_FILE = new File(Utils.fmt("/sbin/.core/db-%d/magisk.db", Const.USER_ID));
        Context de = Build.VERSION.SDK_INT >= Build.VERSION_CODES.N
                ? mm.createDeviceProtectedStorageContext() : mm;
        if (!DB_FILE.canWrite()) {
            if (!Shell.rootAccess()) {
                // We don't want the app to crash, create a db and return
                return mm.openOrCreateDatabase("su.db", Context.MODE_PRIVATE, null);
            }
            mm.loadMagiskInfo();
            // Cleanup
            Shell.Sync.su("db_clean " + Const.USER_ID);
            if (mm.magiskVersionCode < Const.MAGISK_VER.FBE_AWARE) {
                // Super old legacy mode
                return mm.openOrCreateDatabase("su.db", Context.MODE_PRIVATE, null);
            } else if (mm.magiskVersionCode < Const.MAGISK_VER.HIDDEN_PATH) {
                // Legacy mode with FBE aware
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                    de.moveDatabaseFrom(mm, "su.db");
                }
                return de.openOrCreateDatabase("su.db", Context.MODE_PRIVATE, null);
            } else {
                // Global database
                final SuFile GLOBAL_DB = new SuFile("/data/adb/magisk.db");
                mm.deleteDatabase("su.db");
                de.deleteDatabase("su.db");
                if (mm.magiskVersionCode < Const.MAGISK_VER.SEPOL_REFACTOR) {
                    // We need some additional policies on old versions
                    Shell.Sync.su("db_sepatch");
                }
                if (!GLOBAL_DB.exists()) {
                    Shell.Sync.su("db_init");
                    SQLiteDatabase.openOrCreateDatabase(GLOBAL_DB, null).close();
                    Shell.Sync.su("db_restore");
                }
            }
            Shell.Sync.su("db_setup " + Process.myUid());
        }
        // Not using legacy mode, open the mounted global DB
        return SQLiteDatabase.openOrCreateDatabase(DB_FILE, null);
    }

    public void onUpgrade(SQLiteDatabase db, int oldVersion) {
        if (oldVersion == 0) {
            createTables(db);
            oldVersion = 3;
        }
        if (oldVersion == 1) {
            // We're dropping column app_name, rename and re-construct table
            db.execSQL(Utils.fmt("ALTER TABLE %s RENAME TO %s_old", POLICY_TABLE));

            // Create the new tables
            createTables(db);

            // Migrate old data to new tables
            db.execSQL(Utils.fmt("INSERT INTO %s SELECT " +
                            "uid, package_name, policy, until, logging, notification FROM %s_old",
                    POLICY_TABLE, POLICY_TABLE));
            db.execSQL(Utils.fmt("DROP TABLE %s_old", POLICY_TABLE));

            MagiskManager.get().deleteDatabase("sulog.db");
            ++oldVersion;
        }
        if (oldVersion == 2) {
            db.execSQL(Utils.fmt("UPDATE %s SET time=time*1000", LOG_TABLE));
            ++oldVersion;
        }
        if (oldVersion == 3) {
            db.execSQL(Utils.fmt("CREATE TABLE IF NOT EXISTS %s (key TEXT, value TEXT, PRIMARY KEY(key))", STRINGS_TABLE));
            ++oldVersion;
        }
        if (oldVersion == 4) {
            db.execSQL(Utils.fmt("UPDATE %s SET uid=uid%%100000", POLICY_TABLE));
            ++oldVersion;
        }
    }

    // Remove everything, we do not support downgrade
    public void onDowngrade(SQLiteDatabase db) {
        MagiskManager.toast(R.string.su_db_corrupt, Toast.LENGTH_LONG);
        db.execSQL("DROP TABLE IF EXISTS " + POLICY_TABLE);
        db.execSQL("DROP TABLE IF EXISTS " + LOG_TABLE);
        db.execSQL("DROP TABLE IF EXISTS " + SETTINGS_TABLE);
        db.execSQL("DROP TABLE IF EXISTS " + STRINGS_TABLE);
        onUpgrade(db, 0);
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

    public void clearOutdated() {
        // Clear outdated policies
        db.delete(POLICY_TABLE, Utils.fmt("until > 0 AND until < %d", System.currentTimeMillis() / 1000), null);
        // Clear outdated logs
        db.delete(LOG_TABLE, Utils.fmt("time < %d", System.currentTimeMillis() - MagiskManager.get().suLogTimeout * 86400000), null);
    }

    public void deletePolicy(Policy policy) {
        deletePolicy(policy.uid);
    }

    public void deletePolicy(String pkg) {
        db.delete(POLICY_TABLE, "package_name=?", new String[] { pkg });
    }

    public void deletePolicy(int uid) {
        db.delete(POLICY_TABLE, Utils.fmt("uid=%d", uid), null);
    }

    public Policy getPolicy(int uid) {
        Policy policy = null;
        try (Cursor c = db.query(POLICY_TABLE, null, Utils.fmt("uid=%d", uid), null, null, null, null)) {
            if (c.moveToNext()) {
                policy = new Policy(c, pm);
            }
        } catch (PackageManager.NameNotFoundException e) {
            deletePolicy(uid);
            return null;
        }
        return policy;
    }

    public void addPolicy(Policy policy) {
        db.replace(POLICY_TABLE, null, policy.getContentValues());
    }

    public void updatePolicy(Policy policy) {
        db.update(POLICY_TABLE, policy.getContentValues(), Utils.fmt("uid=%d", policy.uid), null);
    }

    public List<Policy> getPolicyList(PackageManager pm) {
        try (Cursor c = db.query(POLICY_TABLE, null, Utils.fmt("uid/100000=%d", Const.USER_ID),
                null, null, null, null)) {
            List<Policy> ret = new ArrayList<>(c.getCount());
            while (c.moveToNext()) {
                try {
                    Policy policy = new Policy(c, pm);
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
        try (Cursor c = db.query(LOG_TABLE, new String[] { "time" }, Utils.fmt("from_uid/100000=%d", Const.USER_ID),
                null, null, null, "time DESC")) {
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
        return db.query(LOG_TABLE, null, Utils.fmt("from_uid/100000=%d", Const.USER_ID),
                null, null, null, "time DESC");
    }

    public void addLog(SuLogEntry log) {
        db.insert(LOG_TABLE, null, log.getContentValues());
    }

    public void clearLogs() {
        db.delete(LOG_TABLE, null, null);
    }

    public void setSettings(String key, int value) {
        ContentValues data = new ContentValues();
        data.put("key", key);
        data.put("value", value);
        db.replace(SETTINGS_TABLE, null, data);
    }

    public int getSettings(String key, int defaultValue) {
        int value = defaultValue;
        try (Cursor c = db.query(SETTINGS_TABLE, null, "key=?",new String[] { key }, null, null, null)) {
            if (c.moveToNext()) {
                value = c.getInt(c.getColumnIndex("value"));
            }
        }
        return value;
    }

    public void setStrings(String key, String value) {
        if (value == null) {
            db.delete(STRINGS_TABLE, "key=?", new String[] { key });
        } else {
            ContentValues data = new ContentValues();
            data.put("key", key);
            data.put("value", value);
            db.replace(STRINGS_TABLE, null, data);
        }
    }

    public String getStrings(String key, String defaultValue) {
        String value = defaultValue;
        try (Cursor c = db.query(STRINGS_TABLE, null, "key=?",new String[] { key }, null, null, null)) {
            if (c.moveToNext()) {
                value = c.getString(c.getColumnIndex("value"));
            }
        }
        return value;
    }
}
