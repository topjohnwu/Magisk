package com.topjohnwu.magisk;

import android.annotation.SuppressLint;
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
import com.topjohnwu.magisk.container.Module;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.database.SuDatabaseHelper;
import com.topjohnwu.magisk.services.UpdateCheckService;
import com.topjohnwu.magisk.superuser.SuReceiver;
import com.topjohnwu.magisk.superuser.SuRequestActivity;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;

import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.ExecutionException;

public class MagiskManager extends Application {

    // Global weak reference to self
    private static WeakReference<MagiskManager> weakSelf;

    public static final String MAGISK_DISABLE_FILE = "/cache/.disable_magisk";
    public static final String MAGISK_HOST_FILE = "/magisk/.core/hosts";
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
    public static final int UPDATE_SERVICE_VER = 1;

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
    public String bootBlock = null;
    public boolean isSuClient = false;
    public String suVersion = null;
    public boolean disabled;
    public int snet_version;
    public int updateServiceVersion;

    // Data
    public Map<String, Module> moduleMap;
    public List<String> blockList;
    public List<Locale> locales;

    // Configurations
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

    public MagiskManager() {
        weakSelf = new WeakReference<>(this);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        prefs = PreferenceManager.getDefaultSharedPreferences(this);

        if (Utils.getDatabasePath(this, SuDatabaseHelper.DB_NAME).exists()) {
            // Don't migrate yet, wait and check Magisk version
            suDB = new SuDatabaseHelper(this);
        } else {
            suDB = new SuDatabaseHelper();
        }

        repoDB = new RepoDatabaseHelper(this);
        defaultLocale = Locale.getDefault();
        setLocale();
        loadConfig();
    }

    public static MagiskManager get() {
        return weakSelf.get();
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
        snet_version = prefs.getInt("snet_version", -1);
        updateServiceVersion = prefs.getInt("update_service_version", -1);
    }

    public static void toast(String msg, int duration) {
        mHandler.post(() -> Toast.makeText(weakSelf.get(), msg, duration).show());
    }

    public static void toast(int resId, int duration) {
        mHandler.post(() -> Toast.makeText(weakSelf.get(), resId, duration).show());
    }

    @SuppressLint("StaticFieldLeak")
    public void startup() {
        if (started)
            return;
        started = true;

        // Dynamic detect all locales
        new ParallelTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                locales = Utils.getAvailableLocale();
                return null;
            }
            @Override
            protected void onPostExecute(Void aVoid) {
                localeDone.publish();
            }
        }.exec();

        // Create notification channel on Android O
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(NOTIFICATION_CHANNEL,
                    getString(R.string.magisk_updates), NotificationManager.IMPORTANCE_DEFAULT);
            getSystemService(NotificationManager.class).createNotificationChannel(channel);
        }

        getMagiskInfo();

        // Magisk working as expected
        if (Shell.rootAccess() && magiskVersionCode > 0) {
            // Load utility shell scripts
            try (InputStream in  = getAssets().open(Utils.UTIL_FUNCTIONS)) {
                shell.loadInputStream(in);
            } catch (IOException e) {
                e.printStackTrace();
            }

            LoadModules loadModuleTask = new LoadModules();

            if (Utils.checkNetworkStatus()) {
                // Make sure we have busybox
                if (!Utils.itemExist(BUSYBOXPATH + "/busybox")) {
                    try {
                        // Force synchronous, make sure we have busybox to use
                        new DownloadBusybox().exec().get();
                    } catch (InterruptedException | ExecutionException e) {
                        e.printStackTrace();
                    }
                }

                // Fire update check
                new CheckUpdates().exec();

                // Add repo update check
                loadModuleTask.setCallBack(() -> new UpdateRepos(false).exec());
            }

            // Root shell initialization
            Shell.su_raw(
                    "export PATH=" + BUSYBOXPATH + ":$PATH",
                    "mount_partitions",
                    "BOOTIMAGE=",
                    "find_boot_image",
                    "migrate_boot_backup"
            );
            List<String> ret = Shell.su("echo \"$BOOTIMAGE\"");
            if (Utils.isValidShellResponse(ret)) {
                bootBlock = ret.get(0);
            } else {
                blockList = Shell.su("find /dev/block -type b | grep -vE 'dm|ram|loop'");
            }

            // Setup suDB
            SuDatabaseHelper.setupSuDB();

            // Add update checking service
            if (UPDATE_SERVICE_VER > updateServiceVersion) {
                ComponentName service = new ComponentName(this, UpdateCheckService.class);
                JobInfo info = new JobInfo.Builder(UPDATE_SERVICE_ID, service)
                        .setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY)
                        .setPersisted(true)
                        .setPeriodic(8 * 60 * 60 * 1000)
                        .build();
                ((JobScheduler) getSystemService(Context.JOB_SCHEDULER_SERVICE)).schedule(info);
                updateServiceVersion = UPDATE_SERVICE_VER;
            }

            // Fire asynctasks
            loadModuleTask.exec();
        }

        // Write back default values
        prefs.edit()
                .putBoolean("dark_theme", isDarkTheme)
                .putBoolean("magiskhide", magiskHide)
                .putBoolean("notification", updateNotification)
                .putBoolean("hosts", Utils.itemExist(MAGISK_HOST_FILE))
                .putBoolean("disable", Utils.itemExist(MAGISK_DISABLE_FILE))
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
                .putInt("update_service_version", updateServiceVersion)
                .apply();
    }

    public void getMagiskInfo() {
        List<String> ret;
        ret = Shell.sh("su -v");
        if (Utils.isValidShellResponse(ret)) {
            suVersion = ret.get(0);
            isSuClient = suVersion.toUpperCase().contains("MAGISK");
        }
        ret = Shell.sh("magisk -v");
        if (!Utils.isValidShellResponse(ret)) {
            ret = Shell.sh("getprop magisk.version");
            if (Utils.isValidShellResponse(ret)) {
                try {
                    magiskVersionString = ret.get(0);
                    magiskVersionCode = (int) Double.parseDouble(ret.get(0)) * 10;
                } catch (NumberFormatException ignored) {}
            }
        } else {
            magiskVersionString = ret.get(0).split(":")[0];
            ret = Shell.sh("magisk -V");
            try {
                magiskVersionCode = Integer.parseInt(ret.get(0));
            } catch (NumberFormatException ignored) {}
        }
        ret = Shell.sh("getprop " + DISABLE_INDICATION_PROP);
        try {
            disabled = Utils.isValidShellResponse(ret) && Integer.parseInt(ret.get(0)) != 0;
        } catch (NumberFormatException e) {
            disabled = false;
        }
        if (magiskVersionCode > 1435) {
            ret = Shell.su("resetprop -p " + MAGISKHIDE_PROP);
        } else {
            ret = Shell.sh("getprop " + MAGISKHIDE_PROP);
        }
        try {
            magiskHide = !Utils.isValidShellResponse(ret) || Integer.parseInt(ret.get(0)) != 0;
        } catch (NumberFormatException e) {
            magiskHide = true;
        }
    }
}
