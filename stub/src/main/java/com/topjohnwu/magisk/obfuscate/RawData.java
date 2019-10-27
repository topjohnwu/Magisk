package com.topjohnwu.magisk.obfuscate;

public class RawData {
    public static String appName() {
        return "Magisk Manager";
    }

    public static String urlBase() {
        return "https://raw.githubusercontent.com/topjohnwu/magisk_files/";
    }

    public static String canary() {
        return "canary/debug.json";
    }

    public static String stable() {
        return "master/stable.json";
    }
}
