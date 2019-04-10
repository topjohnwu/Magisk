package com.topjohnwu.magisk;

import android.os.Environment;
import android.os.Process;

import java.io.File;

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

    public static final String TMP_FOLDER_PATH = "/dev/tmp";
    public static final String MAGISK_LOG = "/cache/magisk.log";
    public static final String MANAGER_CONFIGS = ".tmp.magisk.config";

    // Versions
    public static final int UPDATE_SERVICE_VER = 1;
    public static final int SNET_EXT_VER = 12;

    public static final int USER_ID = Process.myUid() / 100000;

    public static final class MAGISK_VER {
        public static final int MIN_SUPPORT = 18000;
    }

    public static class ID {
        public static final int FETCH_ZIP = 2;
        public static final int SELECT_BOOT = 3;

        // notifications
        public static final int MAGISK_UPDATE_NOTIFICATION_ID = 4;
        public static final int APK_UPDATE_NOTIFICATION_ID = 5;
        public static final int DTBO_NOTIFICATION_ID = 7;
        public static final int HIDE_MANAGER_NOTIFICATION_ID = 8;
        public static final String UPDATE_NOTIFICATION_CHANNEL = "update";
        public static final String PROGRESS_NOTIFICATION_CHANNEL = "progress";
        public static final String CHECK_MAGISK_UPDATE_WORKER_ID = "magisk_update";
    }

    public static class Url {
        private static String getRaw(String where, String name) {
            return String.format("https://raw.githubusercontent.com/topjohnwu/magisk_files/%s/%s", where, name);
        }
        public static final String STABLE_URL = getRaw("master", "stable.json");
        public static final String BETA_URL = getRaw("master", "beta.json");
        public static final String CANARY_URL = getRaw("master", "canary_builds/release.json");
        public static final String CANARY_DEBUG_URL = getRaw("master", "canary_builds/canary.json");
        public static final String REPO_URL = "https://api.github.com/users/Magisk-Modules-Repo/repos?per_page=100&sort=pushed&page=%d";
        public static final String FILE_URL = "https://raw.githubusercontent.com/Magisk-Modules-Repo/%s/master/%s";
        public static final String ZIP_URL = "https://github.com/Magisk-Modules-Repo/%s/archive/master.zip";
        public static final String MODULE_INSTALLER = "https://raw.githubusercontent.com/topjohnwu/Magisk/master/scripts/module_installer.sh";
        public static final String PAYPAL_URL = "https://www.paypal.me/topjohnwu";
        public static final String PATREON_URL = "https://www.patreon.com/topjohnwu";
        public static final String TWITTER_URL = "https://twitter.com/topjohnwu";
        public static final String XDA_THREAD = "http://forum.xda-developers.com/showthread.php?t=3432382";
        public static final String SOURCE_CODE_URL = "https://github.com/topjohnwu/Magisk";
        public static final String SNET_URL = getRaw("b66b1a914978e5f4c4bbfd74a59f4ad371bac107", "snet.apk");
        public static final String BOOTCTL_URL = getRaw("9c5dfc1b8245c0b5b524901ef0ff0f8335757b77", "bootctl");
    }

    public static class Key {
        // others
        public static final String LINK_KEY = "Link";
        public static final String IF_NONE_MATCH = "If-None-Match";
        // intents
        public static final String OPEN_SECTION = "section";
        public static final String INTENT_SET_NAME = "filename";
        public static final String INTENT_SET_LINK = "link";
        public static final String FLASH_ACTION = "action";
        public static final String BROADCAST_MANAGER_UPDATE = "manager_update";
        public static final String BROADCAST_REBOOT = "reboot";
    }

    public static class Value {
        public static final String FLASH_ZIP = "flash";
        public static final String PATCH_FILE = "patch";
        public static final String FLASH_MAGISK = "magisk";
        public static final String FLASH_INACTIVE_SLOT = "slot";
        public static final String UNINSTALL = "uninstall";
    }


}
