package com.topjohnwu.magisk.database;

import android.content.ContentValues;
import android.content.Context;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.os.Build;
import android.text.TextUtils;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.container.Policy;
import com.topjohnwu.magisk.container.SuLogEntry;
import com.topjohnwu.magisk.utils.Const;
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
    public static boolean verified = false;

    private static final int DATABASE_VER = 5;
    private static final String POLICY_TABLE = "policies";
    private static final String LOG_TABLE = "logs";
    private static final String SETTINGS_TABLE = "settings";
    private static final String STRINGS_TABLE = "strings";
    private static final File GLOBAL_DB = new File("/data/adb/magisk.db");

    private Context mContext;
    private PackageManager pm;
    private SQLiteDatabase mDb;

    private static Context initDB(boolean verify) {
        Context context, de = null;
        MagiskManager ce = MagiskManager.get();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            de = ce.createDeviceProtectedStorageContext();
            File ceDB = Utils.getDB(ce, DB_NAME);
            if (ceDB.exists()) {
                context = ce;
            } else {
                context = de;
            }
        } else {
            context = ce;
        }

        File db = Utils.getDB(context, DB_NAME);
        if (!verify) {
            if (db.length() == 0) {
                ce.loadMagiskInfo();
                // Continue verification
            } else {
                return context;
            }
        }

        // Encryption storage
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            if (ce.magiskVersionCode < 1410) {
                if (context == de) {
                    ce.moveDatabaseFrom(de, DB_NAME);
                    context = ce;
                }
            } else {
                if (context == ce) {
                    de.moveDatabaseFrom(ce, DB_NAME);
                    context = de;
                }
            }
        }

        if (!Shell.rootAccess())
            return context;

        // Verify the global db matches the local with the same inode
        if (ce.magiskVersionCode >= 1450 && ce.magiskVersionCode < 1460) {
            // v14.5 global su db
            File OLD_GLOBAL_DB = new File(context.getFilesDir().getParentFile().getParentFile(), "magisk.db");
            verified = TextUtils.equals(Utils.checkInode(OLD_GLOBAL_DB), Utils.checkInode(db));
            if (!verified) {
                context.deleteDatabase(DB_NAME);
                db.getParentFile().mkdirs();
                Shell.su(Utils.fmt("magisk --clone-attr %s %s; chmod 600 %s; ln %s %s",
                        context.getFilesDir(), OLD_GLOBAL_DB, OLD_GLOBAL_DB, OLD_GLOBAL_DB, db));
                verified = TextUtils.equals(Utils.checkInode(OLD_GLOBAL_DB), Utils.checkInode(db));
            }
        } else if (ce.magiskVersionCode >= 1464) {
            // New global su db
            Shell.su(Utils.fmt("mkdir %s 2>/dev/null; chmod 700 %s", GLOBAL_DB.getParent(), GLOBAL_DB.getParent()));
            if (!Utils.itemExist(GLOBAL_DB)) {
                Utils.javaCreateFile(db);
                Shell.su(Utils.fmt("cp -af %s %s; rm -f %s*", db, GLOBAL_DB, db));
            }
            verified = TextUtils.equals(Utils.checkInode(GLOBAL_DB), Utils.checkInode(db));
            if (!verified) {
                context.deleteDatabase(DB_NAME);
                Utils.javaCreateFile(db);
                Shell.su(Utils.fmt(
                        "chown 0.0 %s; chmod 666 %s; chcon u:object_r:su_file:s0 %s;" +
                                "mount -o bind %s %s",
                        GLOBAL_DB, GLOBAL_DB, GLOBAL_DB, GLOBAL_DB, db));
                verified = TextUtils.equals(Utils.checkInode(GLOBAL_DB), Utils.checkInode(db));
            }
        }
        return context;
    }

    public SuDatabaseHelper() {
        this(true);
    }

    public SuDatabaseHelper(boolean verify) {
        this(initDB(verify));
    }

    private SuDatabaseHelper(Context context) {
        super(context, DB_NAME, null, DATABASE_VER);
        mContext = context;
        pm = context.getPackageManager();
        mDb = getWritableDatabase();
        cleanup();

        if (context.getPackageName().equals(Const.ORIG_PKG_NAME)) {
            String pkg = getStrings(Const.Key.SU_REQUESTER, null);
            if (pkg != null) {
                Utils.uninstallPkg(pkg);
                setStrings(Const.Key.SU_REQUESTER, null);
            }
        }
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

    public void cleanup() {
        // Clear outdated policies
        mDb.delete(POLICY_TABLE, Utils.fmt("until > 0 AND until < %d", System.currentTimeMillis() / 1000), null);
        // Clear outdated logs
        mDb.delete(LOG_TABLE, Utils.fmt("time < %d", System.currentTimeMillis() - MagiskManager.get().suLogTimeout * 86400000), null);
    }

    public void deletePolicy(Policy policy) {
        deletePolicy(policy.uid);
    }

    public void deletePolicy(String pkg) {
        mDb.delete(POLICY_TABLE, "package_name=?", new String[] { pkg });
    }

    public void deletePolicy(int uid) {
        mDb.delete(POLICY_TABLE, Utils.fmt("uid=%d", uid), null);
    }

    public Policy getPolicy(int uid) {
        Policy policy = null;
        try (Cursor c = mDb.query(POLICY_TABLE, null, Utils.fmt("uid=%d", uid), null, null, null, null)) {
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
        mDb.replace(POLICY_TABLE, null, policy.getContentValues());
    }

    public void updatePolicy(Policy policy) {
        mDb.update(POLICY_TABLE, policy.getContentValues(), Utils.fmt("uid=%d", policy.uid), null);
    }

    public List<Policy> getPolicyList(PackageManager pm) {
        try (Cursor c = mDb.query(POLICY_TABLE, null, Utils.fmt("uid/100000=%d", Const.USER_ID),
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
        try (Cursor c = mDb.query(LOG_TABLE, new String[] { "time" }, Utils.fmt("from_uid/100000=%d", Const.USER_ID),
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
        return mDb.query(LOG_TABLE, null, Utils.fmt("from_uid/100000=%d", Const.USER_ID),
                null, null, null, "time DESC");
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
