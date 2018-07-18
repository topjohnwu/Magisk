package com.topjohnwu.magisk;

import android.app.job.JobInfo;
import android.app.job.JobScheduler;
import android.content.ComponentName;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.util.Xml;

import com.topjohnwu.magisk.components.Application;
import com.topjohnwu.magisk.container.Module;
import com.topjohnwu.magisk.database.MagiskDatabaseHelper;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.services.UpdateCheckService;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.magisk.utils.ShellInitializer;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.io.SuFile;
import com.topjohnwu.superuser.io.SuFileInputStream;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.File;
import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Locale;
import java.util.Map;

public class MagiskManager extends Application implements Shell.Container {

    // Topics
    public final Topic magiskHideDone = new Topic();
    public final Topic reloadActivity = new Topic();
    public final Topic moduleLoadDone = new Topic();
    public final Topic repoLoadDone = new Topic();
    public final Topic updateCheckDone = new Topic();
    public final Topic safetyNetDone = new Topic();
    public final Topic localeDone = new Topic();

    // Info
    public boolean hasInit = false;
    public String magiskVersionString;
    public int magiskVersionCode = -1;
    public String remoteMagiskVersionString;
    public int remoteMagiskVersionCode = -1;
    public String remoteManagerVersionString;
    public int remoteManagerVersionCode = -1;

    public String magiskLink;
    public String magiskNoteLink;
    public String managerLink;
    public String managerNoteLink;
    public String uninstallerLink;

    public boolean keepVerity = false;
    public boolean keepEnc = false;

    // Data
    public Map<String, Module> moduleMap;
    public List<Locale> locales;

    public boolean magiskHide;
    public boolean isDarkTheme;
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
    public int repoOrder;

    // Global resources
    public SharedPreferences prefs;
    public MagiskDatabaseHelper mDB;
    public RepoDatabaseHelper repoDB;

    private volatile Shell mShell;

    public MagiskManager() {
        weakSelf = new WeakReference<>(this);
        Shell.setContainer(this);
    }

    @Nullable
    @Override
    public Shell getShell() {
        return mShell;
    }

    @Override
    public void setShell(@Nullable Shell shell) {
        mShell = shell;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        Shell.setFlags(Shell.FLAG_MOUNT_MASTER);
        Shell.verboseLogging(BuildConfig.DEBUG);
        Shell.setInitializer(ShellInitializer.class);

        prefs = PreferenceManager.getDefaultSharedPreferences(this);
        mDB = MagiskDatabaseHelper.getInstance(this);

        String pkg = mDB.getStrings(Const.Key.SU_MANAGER, null);
        if (pkg != null && getPackageName().equals(Const.ORIG_PKG_NAME)) {
            mDB.setStrings(Const.Key.SU_MANAGER, null);
            RootUtils.uninstallPkg(pkg);
        }
        if (TextUtils.equals(pkg, getPackageName())) {
            try {
                // We are the manager, remove com.topjohnwu.magisk as it could be malware
                getPackageManager().getApplicationInfo(Const.ORIG_PKG_NAME, 0);
                RootUtils.uninstallPkg(Const.ORIG_PKG_NAME);
            } catch (PackageManager.NameNotFoundException ignored) {}
        }

        setLocale();
        loadConfig();
    }

    public static MagiskManager get() {
        return (MagiskManager) weakSelf.get();
    }

    public void setLocale() {
        localeConfig = prefs.getString(Const.Key.LOCALE, "");
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
        // su
        suRequestTimeout = Utils.getPrefsInt(prefs, Const.Key.SU_REQUEST_TIMEOUT, Const.Value.timeoutList[2]);
        suResponseType = Utils.getPrefsInt(prefs, Const.Key.SU_AUTO_RESPONSE, Const.Value.SU_PROMPT);
        suNotificationType = Utils.getPrefsInt(prefs, Const.Key.SU_NOTIFICATION, Const.Value.NOTIFICATION_TOAST);
        suAccessState = mDB.getSettings(Const.Key.ROOT_ACCESS, Const.Value.ROOT_ACCESS_APPS_AND_ADB);
        multiuserMode = mDB.getSettings(Const.Key.SU_MULTIUSER_MODE, Const.Value.MULTIUSER_MODE_OWNER_ONLY);
        suNamespaceMode = mDB.getSettings(Const.Key.SU_MNT_NS, Const.Value.NAMESPACE_MODE_REQUESTER);

        // config
        isDarkTheme = prefs.getBoolean(Const.Key.DARK_THEME, false);
        updateChannel = Utils.getPrefsInt(prefs, Const.Key.UPDATE_CHANNEL, Const.Value.STABLE_CHANNEL);
        bootFormat = prefs.getString(Const.Key.BOOT_FORMAT, ".img");
        repoOrder = prefs.getInt(Const.Key.REPO_ORDER, Const.Value.ORDER_NAME);
    }

    public void writeConfig() {
        prefs.edit()
                .putBoolean(Const.Key.DARK_THEME, isDarkTheme)
                .putBoolean(Const.Key.MAGISKHIDE, magiskHide)
                .putBoolean(Const.Key.HOSTS, Const.MAGISK_HOST_FILE.exists())
                .putBoolean(Const.Key.COREONLY, Const.MAGISK_DISABLE_FILE.exists())
                .putString(Const.Key.SU_REQUEST_TIMEOUT, String.valueOf(suRequestTimeout))
                .putString(Const.Key.SU_AUTO_RESPONSE, String.valueOf(suResponseType))
                .putString(Const.Key.SU_NOTIFICATION, String.valueOf(suNotificationType))
                .putString(Const.Key.ROOT_ACCESS, String.valueOf(suAccessState))
                .putString(Const.Key.SU_MULTIUSER_MODE, String.valueOf(multiuserMode))
                .putString(Const.Key.SU_MNT_NS, String.valueOf(suNamespaceMode))
                .putString(Const.Key.UPDATE_CHANNEL, String.valueOf(updateChannel))
                .putString(Const.Key.LOCALE, localeConfig)
                .putString(Const.Key.BOOT_FORMAT, bootFormat)
                .putInt(Const.Key.UPDATE_SERVICE_VER, Const.UPDATE_SERVICE_VER)
                .putInt(Const.Key.REPO_ORDER, repoOrder)
                .apply();
    }

    public void loadMagiskInfo() {
        try {
            magiskVersionString = ShellUtils.fastCmd("magisk -v").split(":")[0];
            magiskVersionCode = Integer.parseInt(ShellUtils.fastCmd("magisk -V"));
            String s = ShellUtils.fastCmd((magiskVersionCode >= Const.MAGISK_VER.RESETPROP_PERSIST ?
                    "resetprop -p " : "getprop ") + Const.MAGISKHIDE_PROP);
            magiskHide = s == null || Integer.parseInt(s) != 0;
        } catch (Exception ignored) {}
    }

    public void getDefaultInstallFlags() {
        keepVerity = Boolean.parseBoolean(ShellUtils.fastCmd("echo $KEEPVERITY"));
        keepEnc = Boolean.parseBoolean(ShellUtils.fastCmd("echo $KEEPFORCEENCRYPT"));
    }

    public void setupUpdateCheck() {
        JobScheduler scheduler = (JobScheduler) getSystemService(JOB_SCHEDULER_SERVICE);

        if (prefs.getBoolean(Const.Key.CHECK_UPDATES, true)) {
            if (scheduler.getAllPendingJobs().isEmpty() ||
                    Const.UPDATE_SERVICE_VER > prefs.getInt(Const.Key.UPDATE_SERVICE_VER, -1)) {
                ComponentName service = new ComponentName(this, UpdateCheckService.class);
                JobInfo info = new JobInfo.Builder(Const.ID.UPDATE_SERVICE_ID, service)
                        .setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY)
                        .setPersisted(true)
                        .setPeriodic(8 * 60 * 60 * 1000)
                        .build();
                scheduler.schedule(info);
            }
        } else {
            scheduler.cancel(Const.UPDATE_SERVICE_VER);
        }
    }

    public void dumpPrefs() {
        // Flush prefs to disk
        prefs.edit().commit();
        File xml = new File(getFilesDir().getParent() + "/shared_prefs",
                getPackageName() + "_preferences.xml");
        Shell.Sync.su(Utils.fmt("for usr in /data/user/*; do cat %s > ${usr}/%s; done", xml, Const.MANAGER_CONFIGS));
    }

    public void loadPrefs() {
        SuFile config = new SuFile(Utils.fmt("/data/user/%d/%s", Const.USER_ID, Const.MANAGER_CONFIGS));
        if (config.exists()) {
            SharedPreferences.Editor editor = prefs.edit();
            try {
                SuFileInputStream is = new SuFileInputStream(config);
                XmlPullParser parser = Xml.newPullParser();
                parser.setFeature(XmlPullParser.FEATURE_PROCESS_NAMESPACES, false);
                parser.setInput(is, "UTF-8");
                parser.nextTag();
                parser.require(XmlPullParser.START_TAG, null, "map");
                while (parser.next() != XmlPullParser.END_TAG) {
                    if (parser.getEventType() != XmlPullParser.START_TAG)
                        continue;
                    String key = parser.getAttributeValue(null, "name");
                    String value = parser.getAttributeValue(null, "value");
                    switch (parser.getName()) {
                        case "string":
                            parser.require(XmlPullParser.START_TAG, null, "string");
                            editor.putString(key, parser.nextText());
                            parser.require(XmlPullParser.END_TAG, null, "string");
                            break;
                        case "boolean":
                            parser.require(XmlPullParser.START_TAG, null, "boolean");
                            editor.putBoolean(key, Boolean.parseBoolean(value));
                            parser.nextTag();
                            parser.require(XmlPullParser.END_TAG, null, "boolean");
                            break;
                        case "int":
                            parser.require(XmlPullParser.START_TAG, null, "int");
                            editor.putInt(key, Integer.parseInt(value));
                            parser.nextTag();
                            parser.require(XmlPullParser.END_TAG, null, "int");
                            break;
                        case "long":
                            parser.require(XmlPullParser.START_TAG, null, "long");
                            editor.putLong(key, Long.parseLong(value));
                            parser.nextTag();
                            parser.require(XmlPullParser.END_TAG, null, "long");
                            break;
                        case "float":
                            parser.require(XmlPullParser.START_TAG, null, "int");
                            editor.putFloat(key, Float.parseFloat(value));
                            parser.nextTag();
                            parser.require(XmlPullParser.END_TAG, null, "int");
                            break;
                        default:
                            parser.next();
                    }
                }
            } catch (IOException | XmlPullParserException e) {
                e.printStackTrace();
            }
            editor.remove(Const.Key.ETAG_KEY);
            editor.apply();
            loadConfig();
            config.delete();
        }
    }
}
