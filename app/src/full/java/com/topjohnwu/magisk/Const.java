package com.topjohnwu.magisk;

import android.os.Environment;
import android.os.Process;

import java.io.File;
import java.util.Arrays;
import java.util.List;

public class Const {

    public static final String DEBUG_TAG = "MagiskManager";

    // APK content
    public static final String ANDROID_MANIFEST = "AndroidManifest.xml";

    public static final String SU_KEYSTORE_KEY = "su_key";

    // Paths
    public static final String MAGISK_PATH = "/sbin/.magisk/img";
    public static final File EXTERNAL_PATH;
    public static File MAGISK_DISABLE_FILE;

    static {
        MAGISK_DISABLE_FILE = new File("xxx");
        EXTERNAL_PATH = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
        EXTERNAL_PATH.mkdirs();
    }

    public static final String BUSYBOX_PATH = "/sbin/.magisk/busybox";
    public static final String TMP_FOLDER_PATH = "/dev/tmp";
    public static final String MAGISK_LOG = "/cache/magisk.log";
    public static final String MANAGER_CONFIGS = ".tmp.magisk.config";

    // Versions
    public static final int UPDATE_SERVICE_VER = 1;
    public static final int MIN_MODULE_VER = 1500;
    public static final int SNET_EXT_VER = 12;

    /* A list of apps that should not be shown as hide-able */
    public static final List<String> HIDE_BLACKLIST =  Arrays.asList(
            Data.MM().getPackageName(),
            "com.google.android.gms"
    );

    public static final int USER_ID = Process.myUid() / 100000;

    public static final class MAGISK_VER {
        /* Currently no backwards compatibility needed */
    }

    public static class ID {
        public static final int UPDATE_SERVICE_ID = 1;
        public static final int FETCH_ZIP = 2;
        public static final int SELECT_BOOT = 3;
        public static final int ONBOOT_SERVICE_ID = 6;

        // notifications
        public static final int MAGISK_UPDATE_NOTIFICATION_ID = 4;
        public static final int APK_UPDATE_NOTIFICATION_ID = 5;
        public static final int DTBO_NOTIFICATION_ID = 7;
        public static final int HIDE_MANAGER_NOTIFICATION_ID = 8;
        public static final String UPDATE_NOTIFICATION_CHANNEL = "update";
        public static final String PROGRESS_NOTIFICATION_CHANNEL = "progress";
    }

    public static class Url {
        public static final String STABLE_URL = "https://raw.githubusercontent.com/topjohnwu/magisk_files/master/stable.json";
        public static final String BETA_URL = "https://raw.githubusercontent.com/topjohnwu/magisk_files/master/beta.json";
        public static final String REPO_URL = "https://api.github.com/users/Magisk-Modules-Repo/repos?per_page=100&sort=pushed";
        public static final String FILE_URL = "https://raw.githubusercontent.com/Magisk-Modules-Repo/%s/master/%s";
        public static final String ZIP_URL = "https://github.com/Magisk-Modules-Repo/%s/archive/master.zip";
        public static final String PAYPAL_URL = "https://www.paypal.me/topjohnwu";
        public static final String PATREON_URL = "https://www.patreon.com/topjohnwu";
        public static final String TWITTER_URL = "https://twitter.com/topjohnwu";
        public static final String XDA_THREAD = "http://forum.xda-developers.com/showthread.php?t=3432382";
        public static final String SOURCE_CODE_URL = "https://github.com/topjohnwu/Magisk";
        public static final String SNET_URL = "https://raw.githubusercontent.com/topjohnwu/magisk_files/b66b1a914978e5f4c4bbfd74a59f4ad371bac107/snet.apk";
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
        public static final String INTENT_SET_NAME = "filename";
        public static final String INTENT_SET_LINK = "link";
        public static final String FLASH_ACTION = "action";
        public static final String FLASH_SET_BOOT = "boot";
        public static final String BROADCAST_MANAGER_UPDATE = "manager_update";
        public static final String BROADCAST_REBOOT = "reboot";

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
        public static final int SU_PROMPT = 0;
        public static final int SU_AUTO_DENY = 1;
        public static final int SU_AUTO_ALLOW = 2;
        public static final String FLASH_ZIP = "flash";
        public static final String PATCH_BOOT = "patch";
        public static final String FLASH_MAGISK = "magisk";
        public static final String FLASH_INACTIVE_SLOT = "slot";
        public static final String UNINSTALL = "uninstall";
        public static final int[] timeoutList = {0, -1, 10, 20, 30, 60};
        public static final int ORDER_NAME = 0;
        public static final int ORDER_DATE = 1;
    }
}
