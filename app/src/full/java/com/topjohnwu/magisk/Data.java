package com.topjohnwu.magisk;

import android.content.SharedPreferences;
import android.os.Handler;
import android.os.Looper;
import android.util.Xml;

import com.topjohnwu.magisk.components.AboutCardRow;
import com.topjohnwu.magisk.receivers.GeneralReceiver;
import com.topjohnwu.magisk.receivers.ShortcutReceiver;
import com.topjohnwu.magisk.services.OnBootService;
import com.topjohnwu.magisk.services.UpdateCheckService;
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
import java.util.HashMap;
import java.util.Map;

public class Data {
    // Global app instance
    public static WeakReference<MagiskManager> weakApp;
    public static Handler mainHandler = new Handler(Looper.getMainLooper());
    public static Map<Class, Class> classMap = new HashMap<>();

    // Current status
    public static String magiskVersionString;
    public static int magiskVersionCode = -1;
    public static boolean magiskHide;

    // Update Info
    public static String remoteMagiskVersionString;
    public static int remoteMagiskVersionCode = -1;
    public static String magiskLink;
    public static String magiskNoteLink;
    public static String magiskMD5;
    public static String remoteManagerVersionString;
    public static int remoteManagerVersionCode = -1;
    public static String managerLink;
    public static String managerNoteLink;
    public static String uninstallerLink;

    // Install flags
    public static boolean keepVerity = false;
    public static boolean keepEnc = false;

    // Configs
    public static boolean isDarkTheme;
    public static int suRequestTimeout;
    public static int suLogTimeout = 14;
    public static int multiuserState = -1;
    public static int suResponseType;
    public static int suNotificationType;
    public static int updateChannel;
    public static int repoOrder;

    static {
        classMap.put(MagiskManager.class, a.q.class);
        classMap.put(MainActivity.class, a.b.class);
        classMap.put(SplashActivity.class, a.c.class);
        classMap.put(AboutActivity.class, a.d.class);
        classMap.put(DonationActivity.class, a.e.class);
        classMap.put(FlashActivity.class, a.f.class);
        classMap.put(NoUIActivity.class, a.g.class);
        classMap.put(GeneralReceiver.class, a.h.class);
        classMap.put(ShortcutReceiver.class, a.i.class);
        classMap.put(OnBootService.class, a.j.class);
        classMap.put(UpdateCheckService.class, a.k.class);
        classMap.put(AboutCardRow.class, a.l.class);
        classMap.put(SuRequestActivity.class, a.m.class);

    }

    public static void loadMagiskInfo() {
        try {
            magiskVersionString = ShellUtils.fastCmd("magisk -v").split(":")[0];
            magiskVersionCode = Integer.parseInt(ShellUtils.fastCmd("magisk -V"));
            magiskHide = Shell.su("magiskhide --status").exec().isSuccess();
        } catch (NumberFormatException ignored) {}
    }

    public static MagiskManager MM() {
        return weakApp.get();
    }

    public static void exportPrefs() {
        // Flush prefs to disk
        MagiskManager mm = MM();
        mm.prefs.edit().commit();
        File xml = new File(mm.getFilesDir().getParent() + "/shared_prefs",
                mm.getPackageName() + "_preferences.xml");
        Shell.su(Utils.fmt("cat %s > /data/user/0/%s", xml, Const.MANAGER_CONFIGS)).exec();
    }

    public static void importPrefs() {
        MagiskManager mm = MM();
        SuFile config = new SuFile("/data/user/0/" + Const.MANAGER_CONFIGS);
        if (config.exists()) {
            SharedPreferences.Editor editor = mm.prefs.edit();
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

    public static void loadConfig() {
        MagiskManager mm = MM();
        // su
        suRequestTimeout = Utils.getPrefsInt(mm.prefs, Const.Key.SU_REQUEST_TIMEOUT, Const.Value.timeoutList[2]);
        suResponseType = Utils.getPrefsInt(mm.prefs, Const.Key.SU_AUTO_RESPONSE, Const.Value.SU_PROMPT);
        suNotificationType = Utils.getPrefsInt(mm.prefs, Const.Key.SU_NOTIFICATION, Const.Value.NOTIFICATION_TOAST);

        // config
        isDarkTheme = mm.prefs.getBoolean(Const.Key.DARK_THEME, false);
        updateChannel = Utils.getPrefsInt(mm.prefs, Const.Key.UPDATE_CHANNEL, Const.Value.STABLE_CHANNEL);
        repoOrder = mm.prefs.getInt(Const.Key.REPO_ORDER, Const.Value.ORDER_DATE);
    }

    public static void writeConfig() {
        MM().prefs.edit()
                .putBoolean(Const.Key.DARK_THEME, isDarkTheme)
                .putBoolean(Const.Key.MAGISKHIDE, magiskHide)
                .putBoolean(Const.Key.COREONLY, Const.MAGISK_DISABLE_FILE.exists())
                .putString(Const.Key.SU_REQUEST_TIMEOUT, String.valueOf(suRequestTimeout))
                .putString(Const.Key.SU_AUTO_RESPONSE, String.valueOf(suResponseType))
                .putString(Const.Key.SU_NOTIFICATION, String.valueOf(suNotificationType))
                .putString(Const.Key.UPDATE_CHANNEL, String.valueOf(updateChannel))
                .putInt(Const.Key.UPDATE_SERVICE_VER, Const.UPDATE_SERVICE_VER)
                .putInt(Const.Key.REPO_ORDER, repoOrder)
                .apply();
    }
}
