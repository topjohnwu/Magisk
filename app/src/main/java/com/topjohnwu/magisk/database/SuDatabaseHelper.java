package com.topjohnwu.magisk.database;

import android.content.ContentValues;
import android.content.Context;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.database.DatabaseUtils;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.os.Build;
import android.text.TextUtils;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.container.Policy;
import com.topjohnwu.magisk.container.SuLogEntry;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;

public class SuDatabaseHelper extends SQLiteOpenHelper {

    public static final String DB_NAME = "su.db";

    private static final int DATABASE_VER = 5;
    private static final String POLICY_TABLE = "policies";
    private static final String LOG_TABLE = "logs";
    private static final String SETTINGS_TABLE = "settings";
    private static final String STRINGS_TABLE = "strings";

    private static String GLOBAL_DB;

    private Context mContext;
    private PackageManager pm;
    private SQLiteDatabase mDb;

    private static Context preProcess() {
        Context context;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            Context ce = MagiskManager.get();
            context = ce.createDeviceProtectedStorageContext();
            File oldDB = Utils.getDatabasePath(ce, DB_NAME);
            if (oldDB.exists()) {
                // Migrate DB path
                context.moveDatabaseFrom(ce, DB_NAME);
            }
        } else {
            context = MagiskManager.get();
        }
        GLOBAL_DB = context.getFilesDir().getParentFile().getParent() + "/magisk.db";
        File db = Utils.getDatabasePath(context, DB_NAME);
        if (!db.exists() && Utils.itemExist(GLOBAL_DB)) {
            // Migrate global DB to ours
            db.getParentFile().mkdirs();
            Shell.su(
                    "magisk --clone-attr " + context.getFilesDir() + " " + GLOBAL_DB,
                    "chmod 660 " + GLOBAL_DB,
                    "ln " + GLOBAL_DB + " " + db
            );
        }
        return context;
    }

    public static void setupSuDB() {
        MagiskManager mm = MagiskManager.get();
        // Check if we need to migrate suDB
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N && mm.magiskVersionCode >= 1410 &&
                Utils.getDatabasePath(mm, SuDatabaseHelper.DB_NAME).exists()) {
            mm.suDB.close();
            mm.suDB = new SuDatabaseHelper();
        }

        File suDbFile = mm.suDB.getDbFile();

        if (!Utils.itemExist(GLOBAL_DB)) {
            // Hard link our DB globally
            Shell.su_raw("ln " + suDbFile + " " + GLOBAL_DB);
        }

        // Check if we are linked globally
        List<String> ret = Shell.sh("ls -l " + suDbFile);
        if (Utils.isValidShellResponse(ret)) {
            try {
                int links = Integer.parseInt(ret.get(0).trim().split("\\s+")[1]);
                if (links < 2) {
                    mm.suDB.close();
                    suDbFile.delete();
                    new File(suDbFile + "-journal").delete();
                    mm.suDB = new SuDatabaseHelper();
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public SuDatabaseHelper() {
        this(preProcess());
    }

    public SuDatabaseHelper(Context context) {
        super(context, DB_NAME, null, DATABASE_VER);
        mContext = context;
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
        try {
            if (oldVersion == 0) {
                createTables(db);
                oldVersion = 3;
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

                File oldDB = Utils.getDatabasePath(MagiskManager.get(), "sulog.db");
                if (oldDB.exists()) {
                    migrateLegacyLogList(oldDB, db);
                    MagiskManager.get().deleteDatabase("sulog.db");
                }
                ++oldVersion;
            }
            if (oldVersion == 2) {
                db.execSQL("UPDATE " + LOG_TABLE + " SET time=time*1000");
                ++oldVersion;
            }
            if (oldVersion == 3) {
                db.execSQL(
                        "CREATE TABLE IF NOT EXISTS " + STRINGS_TABLE + " " +
                                "(key TEXT, value TEXT, PRIMARY KEY(key))");
                ++oldVersion;
            }
            if (oldVersion == 4) {
                db.execSQL("UPDATE " + POLICY_TABLE + " SET uid=uid%100000");
                ++oldVersion;
            }

            if (!Utils.itemExist(GLOBAL_DB)) {
                // Hard link our DB globally
                Shell.su_raw("ln " + getDbFile() + " " + GLOBAL_DB);
            }
        } catch (Exception e) {
            e.printStackTrace();
            onDowngrade(db, DATABASE_VER, 0);
        }
    }

    @Override
    public void onDowngrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        MagiskManager.toast(R.string.su_db_corrupt, Toast.LENGTH_LONG);
        // Remove everything, we do not support downgrade
        db.execSQL("DROP TABLE IF EXISTS " + POLICY_TABLE);
        db.execSQL("DROP TABLE IF EXISTS " + LOG_TABLE);
        db.execSQL("DROP TABLE IF EXISTS " + SETTINGS_TABLE);
        db.execSQL("DROP TABLE IF EXISTS " + STRINGS_TABLE);
        onUpgrade(db, 0, DATABASE_VER);
    }

    public File getDbFile() {
        return mContext.getDatabasePath(DB_NAME);
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
                System.currentTimeMillis() - MagiskManager.get().suLogTimeout * 86400000) });
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
        try (Cursor c = mDb.query(POLICY_TABLE, null, "uid=?", new String[] { String.valueOf(uid % 100000) }, null, null, null)) {
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

    public void setStrings(String key, String value) {
        if (value == null) {
            mDb.delete(STRINGS_TABLE, "key=?", new String[] { key });
        } else {
            ContentValues data = new ContentValues();
            data.put("key", key);
            data.put("value", value);
            mDb.replace(STRINGS_TABLE, null, data);
        }
    }

    public String getStrings(String key, String defaultValue) {
        String value = defaultValue;
        try (Cursor c = mDb.query(STRINGS_TABLE, null, "key=?",new String[] { key }, null, null, null)) {
            if (c.moveToNext()) {
                value = c.getString(c.getColumnIndex("value"));
            }
        }
        return value;
    }
}
