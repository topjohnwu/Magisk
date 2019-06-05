package com.topjohnwu.magisk;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Xml;

import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.io.SuFile;
import com.topjohnwu.superuser.io.SuFileInputStream;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.File;
import java.io.IOException;

import androidx.annotation.Nullable;
import androidx.collection.ArrayMap;

import static com.topjohnwu.magisk.ConfigLeanback.getPrefs;
import static com.topjohnwu.magisk.utils.XAndroidKt.getPackageName;

public final class Config {

    private static final ArrayMap<String, Object> defs = new ArrayMap<>();
    public static int magiskVersionCode = -1;
    // Current status
    public static String magiskVersionString = "";
    // Update Info
    public static String remoteMagiskVersionString = "";
    public static int remoteMagiskVersionCode = -1;
    public static String magiskLink = "";
    public static String magiskNoteLink = "";
    public static String magiskMD5 = "";
    public static String remoteManagerVersionString = "";
    public static int remoteManagerVersionCode = -1;
    public static String managerLink = "";
    public static String managerNoteLink = "";
    public static String uninstallerLink = "";

    // Install flags
    public static boolean keepVerity = false;
    public static boolean keepEnc = false;
    public static boolean recovery = false;

    public static int suLogTimeout = 14;

    public static class Key {
        // su configs
        public static final String ROOT_ACCESS = "root_access";
        public static final String SU_MULTIUSER_MODE = "multiuser_mode";
        public static final String SU_MNT_NS = "mnt_ns";
        public static final String SU_MANAGER = "requester";
        public static final String SU_REQUEST_TIMEOUT = "su_request_timeout";
        public static final String SU_AUTO_RESPONSE = "su_auto_response";
        public static final String SU_NOTIFICATION = "su_notification";
        public static final String SU_REAUTH = "su_reauth";
        public static final String SU_FINGERPRINT = "su_fingerprint";

        // prefs
        public static final String CHECK_UPDATES = "check_update";
        public static final String UPDATE_CHANNEL = "update_channel";
        public static final String CUSTOM_CHANNEL = "custom_channel";
        public static final String LOCALE = "locale";
        public static final String DARK_THEME = "dark_theme";
        public static final String ETAG_KEY = "ETag";
        public static final String REPO_ORDER = "repo_order";
        public static final String SHOW_SYSTEM_APP = "show_system";

        // system state
        public static final String UPDATE_SERVICE_VER = "update_service_version";
        public static final String MAGISKHIDE = "magiskhide";
        public static final String COREONLY = "disable";
    }

    public static class Value {
        public static final int DEFAULT_CHANNEL = -1;
        public static final int STABLE_CHANNEL = 0;
        public static final int BETA_CHANNEL = 1;
        public static final int CUSTOM_CHANNEL = 2;
        public static final int CANARY_CHANNEL = 3;
        public static final int CANARY_DEBUG_CHANNEL = 4;
        public static final int ROOT_ACCESS_DISABLED = 0;
        public static final int ROOT_ACCESS_APPS_ONLY = 1;
        public static final int ROOT_ACCESS_ADB_ONLY = 2;
        public static final int ROOT_ACCESS_APPS_AND_ADB = 3;
        public static final int MULTIUSER_MODE_OWNER_ONLY = 0;
        public static final int MULTIUSER_MODE_OWNER_MANAGED = 1;
        public static final int MULTIUSER_MODE_USER = 2;
        public static final int NAMESPACE_MODE_GLOBAL = 0;
        public static final int NAMESPACE_MODE_REQUESTER = 1;
        public static final int NAMESPACE_MODE_ISOLATE = 2;
        public static final int NO_NOTIFICATION = 0;
        public static final int NOTIFICATION_TOAST = 1;
        public static final int SU_PROMPT = 0;
        public static final int SU_AUTO_DENY = 1;
        public static final int SU_AUTO_ALLOW = 2;
        public static final int[] TIMEOUT_LIST = {0, -1, 10, 20, 30, 60};
        public static final int ORDER_NAME = 0;
        public static final int ORDER_DATE = 1;
    }

    private static boolean magiskHide = false;

    public static void loadMagiskInfo() {
        try {
            magiskVersionString = ShellUtils.fastCmd("magisk -v").split(":")[0];
            magiskVersionCode = Integer.parseInt(ShellUtils.fastCmd("magisk -V"));
            magiskHide = Shell.su("magiskhide --status").exec().isSuccess();
        } catch (NumberFormatException ignored) {
        }
    }

    public static void export() {
        // Flush prefs to disk
        getPrefs().edit().apply();
        Context context = ConfigLeanback.getProtectedContext();
        File xml = new File(context.getFilesDir().getParent() + "/shared_prefs",
                getPackageName() + "_preferences.xml");
        Shell.su(Utils.fmt("cat %s > /data/adb/%s", xml, Const.MANAGER_CONFIGS)).exec();
    }

    private static final int PREF_INT = 0;
    private static final int PREF_STR_INT = 1;
    private static final int PREF_BOOL = 2;
    private static final int PREF_STR = 3;
    private static final int DB_INT = 4;
    private static final int DB_BOOL = 5;
    private static final int DB_STR = 6;

    private static int getConfigType(String key) {
        switch (key) {
            case Key.REPO_ORDER:
                return PREF_INT;

            case Key.SU_REQUEST_TIMEOUT:
            case Key.SU_AUTO_RESPONSE:
            case Key.SU_NOTIFICATION:
            case Key.UPDATE_CHANNEL:
                return PREF_STR_INT;

            case Key.DARK_THEME:
            case Key.SU_REAUTH:
            case Key.CHECK_UPDATES:
            case Key.MAGISKHIDE:
            case Key.COREONLY:
            case Key.SHOW_SYSTEM_APP:
                return PREF_BOOL;

            case Key.CUSTOM_CHANNEL:
            case Key.LOCALE:
            case Key.ETAG_KEY:
                return PREF_STR;

            case Key.ROOT_ACCESS:
            case Key.SU_MNT_NS:
            case Key.SU_MULTIUSER_MODE:
                return DB_INT;

            case Key.SU_FINGERPRINT:
                return DB_BOOL;

            case Key.SU_MANAGER:
                return DB_STR;

            default:
                throw new IllegalArgumentException();
        }
    }

    public static void initialize() {
        SharedPreferences pref = getPrefs();
        SharedPreferences.Editor editor = pref.edit();
        File config = SuFile.open("/data/adb", Const.MANAGER_CONFIGS);
        if (config.exists()) {
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
            editor.remove(Key.ETAG_KEY);
            editor.apply();
            editor = pref.edit();
            config.delete();
        }

        // Set defaults if not set
        setDefs(pref, editor);

        // These settings are from actual device state
        editor.putBoolean(Key.MAGISKHIDE, magiskHide)
                .putBoolean(Key.COREONLY, Const.MAGISK_DISABLE_FILE.exists())
                .putInt(Key.UPDATE_SERVICE_VER, Const.UPDATE_SERVICE_VER)
                .apply();
    }

    @SuppressWarnings("unchecked")
    public static <T> T get(String key) {
        switch (getConfigType(key)) {
            case PREF_INT:
                return (T) (Integer) getPrefs().getInt(key, getDef(key));
            case PREF_STR_INT:
                return (T) (Integer) Utils.getPrefsInt(getPrefs(), key, getDef(key));
            case PREF_BOOL:
                return (T) (Boolean) getPrefs().getBoolean(key, getDef(key));
            case PREF_STR:
                return (T) getPrefs().getString(key, getDef(key));
            case DB_INT:
                return (T) (Integer) ConfigLeanback.get(key, (Integer) getDef(key));
            case DB_BOOL:
                return (T) (Boolean) (ConfigLeanback.get(key, getDef(key) ? 1 : 0) != 0);
            case DB_STR:
                return (T) ConfigLeanback.get(key, getDef(key));
        }
        /* Will never get here (IllegalArgumentException in getConfigType) */
        return null;
    }

    public static void set(String key, Object val) {
        switch (getConfigType(key)) {
            case PREF_INT:
                getPrefs().edit().putInt(key, (int) val).apply();
                break;
            case PREF_STR_INT:
                getPrefs().edit().putString(key, String.valueOf(val)).apply();
                break;
            case PREF_BOOL:
                getPrefs().edit().putBoolean(key, (boolean) val).apply();
                break;
            case PREF_STR:
                getPrefs().edit().putString(key, (String) val).apply();
                break;
            case DB_INT:
                ConfigLeanback.put(key, (int) val);
                break;
            case DB_BOOL:
                ConfigLeanback.put(key, (boolean) val ? 1 : 0);
                break;
            case DB_STR:
                ConfigLeanback.put(key, (String) val);
                break;
        }
    }

    public static void remove(String key) {
        switch (getConfigType(key)) {
            case PREF_INT:
            case PREF_STR_INT:
            case PREF_BOOL:
            case PREF_STR:
                getPrefs().edit().remove(key).apply();
                break;
            case DB_BOOL:
            case DB_INT:
                ConfigLeanback.delete(key);
                break;
            case DB_STR:
                ConfigLeanback.put(key, null);
                break;
        }
    }

    static {
        /* Set default configurations */

        // prefs int
        defs.put(Key.REPO_ORDER, Value.ORDER_DATE);

        // prefs string int
        defs.put(Key.SU_REQUEST_TIMEOUT, 10);
        defs.put(Key.SU_AUTO_RESPONSE, Value.SU_PROMPT);
        defs.put(Key.SU_NOTIFICATION, Value.NOTIFICATION_TOAST);
        defs.put(Key.UPDATE_CHANNEL, Utils.isCanary() ?
                Value.CANARY_DEBUG_CHANNEL : Value.DEFAULT_CHANNEL);

        // prefs bool
        defs.put(Key.CHECK_UPDATES, true);
        defs.put(Key.DARK_THEME, true);
        //defs.put(Key.SU_REAUTH, false);
        //defs.put(Key.SHOW_SYSTEM_APP, false);

        // prefs string
        defs.put(Key.CUSTOM_CHANNEL, "");
        defs.put(Key.LOCALE, "");
        //defs.put(Key.ETAG_KEY, null);

        // db int
        defs.put(Key.ROOT_ACCESS, Value.ROOT_ACCESS_APPS_AND_ADB);
        defs.put(Key.SU_MNT_NS, Value.NAMESPACE_MODE_REQUESTER);
        defs.put(Key.SU_MULTIUSER_MODE, Value.MULTIUSER_MODE_OWNER_ONLY);

        // db bool
        //defs.put(Key.SU_FINGERPRINT, false);

        // db strings
        //defs.put(Key.SU_MANAGER, null);
    }

    @SuppressWarnings("unchecked")
    @Nullable
    private static <T> T getDef(String key) throws IllegalArgumentException {
        Object val = defs.get(key);
        switch (getConfigType(key)) {
            case PREF_INT:
            case DB_INT:
            case PREF_STR_INT:
                return val != null ? (T) val : (T) (Integer) 0;
            case DB_BOOL:
            case PREF_BOOL:
                return val != null ? (T) val : (T) (Boolean) false;
            case DB_STR:
            case PREF_STR:
                return (T) val;
        }
        /* Will never get here (IllegalArgumentException in getConfigType) */
        return null;
    }

    private static void setDefs(SharedPreferences pref, SharedPreferences.Editor editor) {
        for (String key : defs.keySet()) {
            Object value = defs.get(key);
            int type = getConfigType(key);
            switch (type) {
                case DB_INT:
                    editor.putString(key, String.valueOf(ConfigLeanback.get(key, (Integer) value)));
                    continue;
                case DB_STR:
                    editor.putString(key, ConfigLeanback.get(key, String.valueOf(value)));
                    continue;
                case DB_BOOL:
                    int bs = ConfigLeanback.get(key, -1);
                    editor.putBoolean(key, bs < 0 ? (Boolean) value : bs != 0);
                    continue;
            }
            if (pref.contains(key))
                continue;
            switch (type) {
                case PREF_INT:
                    editor.putInt(key, (Integer) value);
                    break;
                case PREF_STR_INT:
                    editor.putString(key, String.valueOf(value));
                    break;
                case PREF_STR:
                    editor.putString(key, (String) value);
                    break;
                case PREF_BOOL:
                    editor.putBoolean(key, (Boolean) value);
                    break;
            }
        }
    }
}
