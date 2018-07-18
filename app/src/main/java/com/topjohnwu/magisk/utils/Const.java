package com.topjohnwu.magisk.utils;

import android.os.Environment;
import android.os.Process;

import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.MagiskManager;

import java.io.File;
import java.util.Arrays;
import java.util.List;

public class Const {

    public static final String DEBUG_TAG = "MagiskManager";
    public static final String ORIG_PKG_NAME = BuildConfig.APPLICATION_ID;
    public static final String MAGISKHIDE_PROP = "persist.magisk.hide";

    // APK content
    public static final String ANDROID_MANIFEST = "AndroidManifest.xml";

    public static final String SU_KEYSTORE_KEY = "su_key";

    // Paths
    public static File MAGISK_PATH;
    public static File MAGISK_DISABLE_FILE;
    public static File MAGISK_HOST_FILE;

    static {
        /* Prevent crashing on unrooted devices */
        MAGISK_PATH = MAGISK_DISABLE_FILE = MAGISK_HOST_FILE = new File("xxx");
    }

    public static final String BUSYBOX_PATH = "/sbin/.core/busybox";
    public static final String TMP_FOLDER_PATH = "/dev/tmp";
    public static final String MAGISK_LOG = "/cache/magisk.log";
    public static final File EXTERNAL_PATH = new File(Environment.getExternalStorageDirectory(), "MagiskManager");
    public static final String MANAGER_CONFIGS = ".tmp.magisk.config";

    // Versions
    public static final int UPDATE_SERVICE_VER = 1;
    public static final int SNET_VER = 8;

    public static int MIN_MODULE_VER() {
        return MagiskManager.get().magiskVersionCode >= MAGISK_VER.REMOVE_LEGACY_LINK ? 1500 : 1400;
    }

    /* A list of apps that should not be shown as hide-able */
    public static final List<String> HIDE_BLACKLIST =  Arrays.asList(
            "android",
            MagiskManager.get().getPackageName(),
            "com.google.android.gms"
    );

    public static final int USER_ID = Process.myUid() / 100000;

    public static final class MAGISK_VER {
        public static final int UNIFIED = 1300;
        public static final int FBE_AWARE = 1410;
        public static final int RESETPROP_PERSIST = 1436;
        public static final int MANAGER_HIDE = 1440;
        public static final int DTBO_SUPPORT = 1446;
        public static final int HIDDEN_PATH = 1460;
        public static final int REMOVE_LEGACY_LINK = 1630;
        public static final int SEPOL_REFACTOR = 1640;
        public static final int FIX_ENV = 1650;
    }

    public static class ID {
        public static final int UPDATE_SERVICE_ID = 1;
        public static final int FETCH_ZIP = 2;
        public static final int SELECT_BOOT = 3;

        // notifications
        public static final int MAGISK_UPDATE_NOTIFICATION_ID = 4;
        public static final int APK_UPDATE_NOTIFICATION_ID = 5;
        public static final int ONBOOT_NOTIFICATION_ID = 6;
        public static final int DTBO_NOTIFICATION_ID = 7;
        public static final String NOTIFICATION_CHANNEL = "magisk_notification";
    }

    public static class Url {
        public static final String STABLE_URL = "https://raw.githubusercontent.com/topjohnwu/MagiskManager/update/stable.json";
        public static final String BETA_URL = "https://raw.githubusercontent.com/topjohnwu/MagiskManager/update/beta.json";
        public static final String SNET_URL = "https://github.com/topjohnwu/MagiskManager/raw/788f01f499714471949613820d43bc364786e85d/snet.apk";
        public static final String REPO_URL = "https://api.github.com/users/Magisk-Modules-Repo/repos?per_page=100&page=%d";
        public static final String FILE_URL = "https://raw.githubusercontent.com/Magisk-Modules-Repo/%s/master/%s";
        public static final String ZIP_URL = "https://github.com/Magisk-Modules-Repo/%s/archive/master.zip";
        public static final String DONATION_URL = "https://www.paypal.me/topjohnwu";
        public static final String XDA_THREAD = "http://forum.xda-developers.com/showthread.php?t=3432382";
        public static final String SOURCE_CODE_URL = "https://github.com/topjohnwu/MagiskManager";
    }


    public static class Key {
        // su
        public static final String ROOT_ACCESS = "root_access";
        public static final String SU_MULTIUSER_MODE = "multiuser_mode";
        public static final String SU_MNT_NS = "mnt_ns";
        public static final String SU_MANAGER = "requester";
        public static final String SU_REQUEST_TIMEOUT = "su_request_timeout";
        public static final String SU_AUTO_RESPONSE = "su_auto_response";
        public static final String SU_NOTIFICATION = "su_notification";
        public static final String SU_REAUTH = "su_reauth";
        public static final String SU_FINGERPRINT = "su_fingerprint";

        // intents
        public static final String OPEN_SECTION = "section";
        public static final String INTENT_SET_FILENAME = "filename";
        public static final String INTENT_SET_LINK = "link";
        public static final String INTENT_PERM = "perm_dialog";
        public static final String FLASH_ACTION = "action";
        public static final String FLASH_SET_BOOT = "boot";

        // others
        public static final String CHECK_UPDATES = "check_update";
        public static final String UPDATE_CHANNEL = "update_channel";
        public static final String CUSTOM_CHANNEL = "custom_channel";
        public static final String BOOT_FORMAT = "boot_format";
        public static final String UPDATE_SERVICE_VER = "update_service_version";
        public static final String APP_VER = "app_version";
        public static final String MAGISKHIDE = "magiskhide";
        public static final String HOSTS = "hosts";
        public static final String COREONLY = "disable";
        public static final String LOCALE = "locale";
        public static final String DARK_THEME = "dark_theme";
        public static final String ETAG_KEY = "ETag";
        public static final String LINK_KEY = "Link";
        public static final String IF_NONE_MATCH = "If-None-Match";
        public static final String REPO_ORDER = "repo_order";
    }


    public static class Value {
        public static final int STABLE_CHANNEL = 0;
        public static final int BETA_CHANNEL = 1;
        public static final int CUSTOM_CHANNEL = 2;
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
        public static final int NOTIFY_NORMAL_LOG = 0;
        public static final int NOTIFY_USER_TOASTS = 1;
        public static final int NOTIFY_USER_TO_OWNER = 2;
        public static final int SU_PROMPT = 0;
        public static final int SU_AUTO_DENY = 1;
        public static final int SU_AUTO_ALLOW = 2;
        public static final String FLASH_ZIP = "flash";
        public static final String PATCH_BOOT = "patch";
        public static final String FLASH_MAGISK = "magisk";
        public static final String FLASH_SECOND_SLOT = "slot";
        public static final String UNINSTALL = "uninstall";
        public static final int[] timeoutList = {0, -1, 10, 20, 30, 60};
        public static final int ORDER_NAME = 0;
        public static final int ORDER_DATE = 1;
    }
}
