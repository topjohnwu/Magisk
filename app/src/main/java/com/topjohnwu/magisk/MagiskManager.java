package com.topjohnwu.magisk;

import android.app.Application;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.job.JobInfo;
import android.app.job.JobScheduler;
import android.content.ComponentName;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Build;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.widget.Toast;

import com.topjohnwu.magisk.asyncs.CheckUpdates;
import com.topjohnwu.magisk.asyncs.DownloadBusybox;
import com.topjohnwu.magisk.asyncs.LoadModules;
import com.topjohnwu.magisk.asyncs.ParallelTask;
import com.topjohnwu.magisk.asyncs.UpdateRepos;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.database.SuDatabaseHelper;
import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.services.UpdateCheckService;
import com.topjohnwu.magisk.superuser.SuReceiver;
import com.topjohnwu.magisk.superuser.SuRequestActivity;
import com.topjohnwu.magisk.utils.SafetyNetHelper;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.ExecutionException;

public class MagiskManager extends Application {

    public static final String MAGISK_DISABLE_FILE = "/cache/.disable_magisk";
    public static final String TMP_FOLDER_PATH = "/dev/tmp";
    public static final String MAGISK_PATH = "/magisk";
    public static final String INTENT_SECTION = "section";
    public static final String INTENT_VERSION = "version";
    public static final String INTENT_LINK = "link";
    public static final String MAGISKHIDE_PROP = "persist.magisk.hide";
    public static final String DISABLE_INDICATION_PROP = "ro.magisk.disable";
    public static final String NOTIFICATION_CHANNEL = "magisk_update_notice";
    public static final String BUSYBOXPATH = "/dev/magisk/bin";
    public static final int UPDATE_SERVICE_ID = 1;

    // Topics
    public final Topic magiskHideDone = new Topic();
    public final Topic reloadActivity = new Topic();
    public final Topic moduleLoadDone = new Topic();
    public final Topic repoLoadDone = new Topic();
    public final Topic updateCheckDone = new Topic();
    public final Topic safetyNetDone = new Topic();
    public final Topic localeDone = new Topic();

    // Info
    public String magiskVersionString;
    public int magiskVersionCode = -1;
    public String remoteMagiskVersionString;
    public int remoteMagiskVersionCode = -1;
    public String magiskLink;
    public String releaseNoteLink;
    public String remoteManagerVersionString;
    public int remoteManagerVersionCode = -1;
    public String managerLink;
    public SafetyNetHelper.Result SNCheckResult;
    public String bootBlock = null;
    public boolean isSuClient = false;
    public String suVersion = null;
    public boolean disabled;

    // Data
    public Map<String, Module> moduleMap;
    public List<String> blockList;
    public List<Locale> locales;

    // Configurations
    public static boolean shellLogging;
    public static boolean devLogging;
    public static Locale locale;
    public static Locale defaultLocale;

    public boolean magiskHide;
    public boolean isDarkTheme;
    public boolean updateNotification;
    public boolean suReauth;
    public int suRequestTimeout;
    public int suLogTimeout = 14;
    public int suAccessState;
    public int multiuserMode;
    public int suResponseType;
    public int suNotificationType;
    public int suNamespaceMode;
    public String localeConfig;
    public int updateChannel;
    public String bootFormat;

    // Global resources
    public SharedPreferences prefs;
    public SuDatabaseHelper suDB;
    public RepoDatabaseHelper repoDB;
    public Shell shell;

    private static Handler mHandler = new Handler();
    private boolean started = false;

    private static class LoadLocale extends ParallelTask<Void, Void, Void> {

        LoadLocale(Context context) {
            super(context);
        }

        @Override
        protected Void doInBackground(Void... voids) {
            getMagiskManager().locales = Utils.getAvailableLocale(getMagiskManager());
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            getMagiskManager().localeDone.publish();
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        prefs = PreferenceManager.getDefaultSharedPreferences(this);

        if (getDatabasePath(SuDatabaseHelper.DB_NAME).exists()) {
            // Don't migrate yet, wait and check Magisk version
            suDB = new SuDatabaseHelper(this);
        } else {
            suDB = new SuDatabaseHelper(Utils.getEncContext(this));
        }

        repoDB = new RepoDatabaseHelper(this);
        defaultLocale = Locale.getDefault();
        setLocale();
        loadConfig();
    }

    public void setLocale() {
        localeConfig = prefs.getString("locale", "");
        if (localeConfig.isEmpty()) {
            locale = defaultLocale;
        } else {
            locale = Locale.forLanguageTag(localeConfig);
        }
        Resources res = getBaseContext().getResources();
        Configuration config = new Configuration(res.getConfiguration());
        config.setLocale(locale);
        res.updateConfiguration(config, res.getDisplayMetrics());
    }

    public void loadConfig() {
        isDarkTheme = prefs.getBoolean("dark_theme", false);
        if (BuildConfig.DEBUG) {
            devLogging = prefs.getBoolean("developer_logging", false);
            shellLogging = prefs.getBoolean("shell_logging", false);
        } else {
            devLogging = false;
            shellLogging = false;
        }

        // su
        suRequestTimeout = Utils.getPrefsInt(prefs, "su_request_timeout", 10);
        suResponseType = Utils.getPrefsInt(prefs, "su_auto_response", SuRequestActivity.PROMPT);
        suNotificationType = Utils.getPrefsInt(prefs, "su_notification", SuReceiver.TOAST);
        suReauth = prefs.getBoolean("su_reauth", false);
        suAccessState = suDB.getSettings(SuDatabaseHelper.ROOT_ACCESS, SuDatabaseHelper.ROOT_ACCESS_APPS_AND_ADB);
        multiuserMode = suDB.getSettings(SuDatabaseHelper.MULTIUSER_MODE, SuDatabaseHelper.MULTIUSER_MODE_OWNER_ONLY);
        suNamespaceMode = suDB.getSettings(SuDatabaseHelper.MNT_NS, SuDatabaseHelper.NAMESPACE_MODE_REQUESTER);

        updateNotification = prefs.getBoolean("notification", true);
        updateChannel = Utils.getPrefsInt(prefs, "update_channel", CheckUpdates.STABLE_CHANNEL);
        bootFormat = prefs.getString("boot_format", ".img");
    }

    public void toast(String msg, int duration) {
        mHandler.post(() -> Toast.makeText(this, msg, duration).show());
    }

    public void toast(int resId, int duration) {
        mHandler.post(() -> Toast.makeText(this, resId, duration).show());
    }

    public void startup() {
        if (started)
            return;
        started = true;

        boolean hasNetwork = Utils.checkNetworkStatus(this);

        getMagiskInfo();

        // Check if we need to migrate suDB
        if (getDatabasePath(SuDatabaseHelper.DB_NAME).exists() && Utils.useFDE(this)) {
            if (magiskVersionCode >= 1410) {
                suDB.close();
                Context de = createDeviceProtectedStorageContext();
                de.moveDatabaseFrom(this, SuDatabaseHelper.DB_NAME);
                suDB = new SuDatabaseHelper(de);
            }
        }

        new LoadLocale(this).exec();

        // Root actions
        if (Shell.rootAccess()) {
            if (hasNetwork && !Utils.itemExist(shell, BUSYBOXPATH + "/busybox")) {
                try {
                    // Force synchronous, make sure we have busybox to use
                    new DownloadBusybox(this).exec().get();
                } catch (InterruptedException | ExecutionException e) {
                    e.printStackTrace();
                }
            }

            File utils = new File(getFilesDir(), Utils.UTIL_FUNCTIONS);

            try (InputStream in  = getAssets().open(Utils.UTIL_FUNCTIONS);
                 OutputStream out = new FileOutputStream(utils)
            ) {
                int read;
                byte[] bytes = new byte[4096];
                while ((read = in.read(bytes)) != -1) {
                    out.write(bytes, 0, read);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }

            shell.su_raw(
                    "export PATH=" + BUSYBOXPATH + ":$PATH",
                    ". " + utils,
                    "mount_partitions",
                    "BOOTIMAGE=",
                    "find_boot_image",
                    "migrate_boot_backup"
            );

            List<String> res = shell.su("echo \"$BOOTIMAGE\"");
            if (Utils.isValidShellResponse(res)) {
                bootBlock = res.get(0);
            } else {
                blockList = shell.su("find /dev/block -type b | grep -vE 'dm|ram|loop'");
            }
        }

        // Write back default values
        prefs.edit()
                .putBoolean("dark_theme", isDarkTheme)
                .putBoolean("magiskhide", magiskHide)
                .putBoolean("notification", updateNotification)
                .putBoolean("hosts", new File("/magisk/.core/hosts").exists())
                .putBoolean("disable", Utils.itemExist(shell, MAGISK_DISABLE_FILE))
                .putBoolean("su_reauth", suReauth)
                .putString("su_request_timeout", String.valueOf(suRequestTimeout))
                .putString("su_auto_response", String.valueOf(suResponseType))
                .putString("su_notification", String.valueOf(suNotificationType))
                .putString("su_access", String.valueOf(suAccessState))
                .putString("multiuser_mode", String.valueOf(multiuserMode))
                .putString("mnt_ns", String.valueOf(suNamespaceMode))
                .putString("update_channel", String.valueOf(updateChannel))
                .putString("locale", localeConfig)
                .putString("boot_format", bootFormat)
                .apply();

        // Create notification channel on Android O
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(NOTIFICATION_CHANNEL,
                    getString(R.string.magisk_updates), NotificationManager.IMPORTANCE_DEFAULT);
            getSystemService(NotificationManager.class).createNotificationChannel(channel);
        }

        LoadModules loadModuleTask = new LoadModules(this);
        // Start update check job
        if (hasNetwork) {
            ComponentName service = new ComponentName(this, UpdateCheckService.class);
            JobInfo jobInfo = new JobInfo.Builder(UPDATE_SERVICE_ID, service)
                    .setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY)
                    .setPersisted(true)
                    .setPeriodic(8 * 60 * 60 * 1000)
                    .build();
            ((JobScheduler) getSystemService(Context.JOB_SCHEDULER_SERVICE)).schedule(jobInfo);
            loadModuleTask.setCallBack(() -> new UpdateRepos(this).exec());
        }
        // Fire asynctasks
        loadModuleTask.exec();

    }

    public void getMagiskInfo() {
        List<String> ret;
        Shell.getShell(this);
        ret = shell.sh("su -v");
        if (Utils.isValidShellResponse(ret)) {
            suVersion = ret.get(0);
            isSuClient = suVersion.toUpperCase().contains("MAGISK");
        }
        ret = shell.sh("magisk -v");
        if (!Utils.isValidShellResponse(ret)) {
            ret = shell.sh("getprop magisk.version");
            if (Utils.isValidShellResponse(ret)) {
                try {
                    magiskVersionString = ret.get(0);
                    magiskVersionCode = (int) Double.parseDouble(ret.get(0)) * 10;
                } catch (NumberFormatException ignored) {}
            }
        } else {
            magiskVersionString = ret.get(0).split(":")[0];
            ret = shell.sh("magisk -V");
            try {
                magiskVersionCode = Integer.parseInt(ret.get(0));
            } catch (NumberFormatException ignored) {}
        }
        ret = shell.sh("getprop " + DISABLE_INDICATION_PROP);
        try {
            disabled = Utils.isValidShellResponse(ret) && Integer.parseInt(ret.get(0)) != 0;
        } catch (NumberFormatException e) {
            disabled = false;
        }
        ret = shell.sh("getprop " + MAGISKHIDE_PROP);
        try {
            magiskHide = !Utils.isValidShellResponse(ret) || Integer.parseInt(ret.get(0)) != 0;
        } catch (NumberFormatException e) {
            magiskHide = true;
        }
    }
}
