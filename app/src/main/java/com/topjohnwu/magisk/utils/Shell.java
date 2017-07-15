package com.topjohnwu.magisk.utils;

import android.content.Context;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Modified by topjohnwu, based on Chainfire's libsuperuser
 */

public class Shell {

    // -1 = problematic/unknown issue; 0 = not rooted; 1 = properly rooted
    public static int rootStatus;

    private final Process rootShell;
    private final DataOutputStream rootSTDIN;
    private final DataInputStream rootSTDOUT;

    private boolean isValid;

    private Shell() {
        Process process;
        try {
            process = Runtime.getRuntime().exec("su");
        } catch (IOException e) {
            // No root
            rootStatus = 0;
            rootShell = null;
            rootSTDIN = null;
            rootSTDOUT = null;
            isValid = false;
            return;
        }

        rootStatus = 1;
        isValid = true;
        rootShell = process;
        rootSTDIN = new DataOutputStream(rootShell.getOutputStream());
        rootSTDOUT = new DataInputStream(rootShell.getInputStream());

        su_raw("umask 022");
        List<String> ret = su("echo -BOC-", "id");

        if (ret.isEmpty()) {
            // Something wrong with root, not allowed?
            rootStatus = -1;
            return;
        }

        for (String line : ret) {
            if (line.contains("uid=")) {
                // id command is working, let's see if we are actually root
                rootStatus = line.contains("uid=0") ? 1 : -1;
                return;
            } else if (!line.contains("-BOC-")) {
                rootStatus = -1;
                return;
            }
        }
    }

    public static Shell getRootShell() {
        return new Shell();
    }

    public static Shell getRootShell(Context context) {
        return Utils.getMagiskManager(context).rootShell;
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
                    Logger.shell(false, write);
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

    public List<String> su(String... commands) {
        if (!isValid) return null;
        List<String> res = new ArrayList<>();
        su(res, commands);
        return res;
    }

    public void su_raw(String... commands) {
        if (!isValid) return;
        synchronized (rootShell) {
            try {
                for (String command : commands) {
                    rootSTDIN.write((command + "\n").getBytes("UTF-8"));
                    rootSTDIN.flush();
                }
            } catch (IOException e) {
                e.printStackTrace();
                rootShell.destroy();
                isValid = false;
            }
        }
    }

    public void su(List<String> output, String... commands) {
        if (!isValid) return;
        try {
            rootShell.exitValue();
            isValid = false;
            return;  // The process is dead, return
        } catch (IllegalThreadStateException ignored) {
            // This should be the expected result
        }
        synchronized (rootShell) {
            StreamGobbler STDOUT = new StreamGobbler(rootSTDOUT, output, true);
            STDOUT.start();
            su_raw(commands);
            su_raw("echo \'-root-done-\'");
            try { STDOUT.join(); } catch (InterruptedException ignored) {}
        }
    }
}
