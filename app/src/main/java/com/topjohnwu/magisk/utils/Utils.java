package com.topjohnwu.magisk.utils;

import java.util.List;
import eu.chainfire.libsuperuser.Shell;

public class Utils {

    public static final String suPath = sh("getprop magisk.supath");
    public static boolean rootAccess = false;

    public static String sh(String... commands) {
        List<String> result = Shell.SH.run(commands);

        StringBuilder builder = new StringBuilder();
        for (String s : result) {
            builder.append(s);
        }

        return builder.toString();
    }

    public static String su(String... commands) {
        List<String> result = Shell.run(Utils.suPath + "/su", commands, null, false);

        StringBuilder builder = new StringBuilder();
        for (String s : result) {
            builder.append(s);
        }

        return builder.toString();
    }

    public static void checkRoot() {
        String [] availableTestCommands = new String[] {"echo -BOC-", "id"};
        List<String> ret = Shell.run(Utils.suPath + "/su", availableTestCommands, null, false);
        if (ret == null)
            return;

        // Taken from libsuperuser

        // this is only one of many ways this can be done

        for (String line : ret) {
            if (line.contains("uid=")) {
                // id command is working, let's see if we are actually root
                rootAccess = line.contains("uid=0");
            } else if (line.contains("-BOC-")) {
                // if we end up here, at least the su command starts some kind
                // of shell, let's hope it has root privileges - no way to know without
                // additional native binaries
                rootAccess = true;
            }
        }
    }

}
