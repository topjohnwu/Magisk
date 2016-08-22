package com.topjohnwu.magisk.ui.utils;

import java.util.List;

import eu.chainfire.libsuperuser.Shell;

public class Utils {

    public static String executeCommand(String... commands) {
        List<String> result = Shell.SU.run(commands);

        StringBuilder builder = new StringBuilder();
        for (String s : result) {
            builder.append(s);
        }

        return builder.toString();
    }

}
