package com.topjohnwu.magisk.obfuscate;

import android.content.res.Resources;

import com.topjohnwu.magisk.R;

public class RawData {

    public static Resources res;

    public static String appName() {
        return "Magisk Manager";
    }

    public static String urlBase() {
        return "https://raw.githubusercontent.com/topjohnwu/magisk_files/";
    }

    public static String debug() {
        return "canary/debug.json";
    }

    public static String canary() {
        return "canary/release.json";
    }

    public static String stable() {
        return "master/stable.json";
    }

    public static String networkError() {
        return res.getString(R.string.no_internet_msg);
    }

    public static String upgradeMsg() {
        return res.getString(R.string.upgrade_msg);
    }

    public static String dling() {
        return res.getString(R.string.dling);
    }
}
