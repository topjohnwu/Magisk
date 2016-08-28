package com.topjohnwu.magisk.utils;

import android.os.AsyncTask;
import android.util.Log;

import java.io.DataOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Modified by topjohnwu, based on Chainfire's libsuperuser
 */

public class Shell {

    // -1 = problematic/unknown issue; 0 = not rooted; 1 = properly rooted; 2 = improperly rooted;
    public static int rootStatus;

    private static Process rootShell;
    private static DataOutputStream rootSTDIN;
    private static StreamGobbler rootSTDOUT;
    private static List<String> rootOutList = new ArrayList<>();

    static {
        init();
    }

    private static void init() {

        try {
            rootShell = Runtime.getRuntime().exec(sh("getprop magisk.supath").get(0) + "/su");
            rootStatus = 1;
        } catch (IOException e) {
            try {
                // Improper root
                rootShell = Runtime.getRuntime().exec("su");
                rootStatus = 2;
            } catch (IOException err) {
                // No root
                rootStatus = 0;
                return;
            }
        }

        rootSTDIN = new DataOutputStream(rootShell.getOutputStream());
        rootSTDOUT = new StreamGobbler(rootShell.getInputStream(), rootOutList);
        rootSTDOUT.start();

        List<String> ret = su("echo -BOC-", "id");
        if (ret == null) {
            // Something wrong with root, not allowed?
            rootStatus = -1;
            return;
        }

        for (String line : ret) {
            if (line.contains("uid=")) {
                // id command is working, let's see if we are actually root
                rootStatus = line.contains("uid=0") ? rootStatus : -1;
                return;
            } else if (!line.contains("-BOC-")) {
                rootStatus = -1;
                return;
            }
        }
    }

    public static boolean rootAccess() {
        return rootStatus > 0;
    }

    public static List<String> sh(String... commands) {
        List<String> res = Collections.synchronizedList(new ArrayList<String>());

        try {
            Process process = Runtime.getRuntime().exec("sh");
            DataOutputStream STDIN = new DataOutputStream(process.getOutputStream());
            StreamGobbler STDOUT = new StreamGobbler(process.getInputStream(), res);

            STDOUT.start();

            try {
                for (String write : commands) {
                    STDIN.write((write + "\n").getBytes("UTF-8"));
                    STDIN.flush();
                }
                STDIN.write("exit\n".getBytes("UTF-8"));
                STDIN.flush();
            } catch (IOException e) {
                if (!e.getMessage().contains("EPIPE")) {
                    throw e;
                }
            }

            process.waitFor();

            try {
                STDIN.close();
            } catch (IOException e) {
                // might be closed already
            }
            STDOUT.join();
            process.destroy();

        } catch (IOException | InterruptedException e) {
            // shell probably not found
            res = null;
        }

        return res;
    }

    public static List<String> su(String... commands) {

        if(!Shell.rootAccess()) return null;

        rootOutList.clear();

        try {
            for (String write : commands) {
                rootSTDIN.write((write + "\n").getBytes("UTF-8"));
                rootSTDIN.flush();
            }
            rootSTDIN.write(("echo \' \'\n").getBytes("UTF-8"));
            rootSTDIN.flush();
            rootSTDIN.write(("echo \'-done-\'\n").getBytes("UTF-8"));
            rootSTDIN.flush();
        } catch (IOException e) {
            if (!e.getMessage().contains("EPIPE")) {
                return null;
            }
        }

        while (true) {
            try {
                // Process terminated, it means the interactive shell cannot be initialized
                rootShell.exitValue();
                return null;
            } catch (IllegalThreadStateException e) {
                // Process still running, gobble output until done
                int end = rootOutList.size() - 1;
                if (rootOutList != null && end > 0) {
                    if (rootOutList.get(end).equals("-done-")) {
                        rootOutList.remove(end);
                        rootOutList.remove(end - 1);
                        break;
                    }
                }
                try { rootSTDOUT.join(100); } catch (InterruptedException err) { return null; }
            }
        }

        return new ArrayList<>(rootOutList);
    }
}
