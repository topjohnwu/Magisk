package com.topjohnwu.magisk.utils;

import android.content.Context;

import com.topjohnwu.magisk.MagiskManager;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * Modified by topjohnwu, based on Chainfire's libsuperuser
 */

public class Shell {

    // -1 = problematic/unknown issue; 0 = not rooted; 1 = properly rooted
    public static int rootStatus;

    private final Process shellProcess;
    private final DataOutputStream STDIN;
    private final DataInputStream STDOUT;

    private boolean isValid;

    private Shell() {
        rootStatus = 1;
        Process process = null;
        DataOutputStream in = null;
        DataInputStream out = null;

        try {
            process = Runtime.getRuntime().exec("su");
            in = new DataOutputStream(process.getOutputStream());
            out = new DataInputStream(process.getInputStream());
        } catch (IOException e) {
            rootStatus = 0;
        }

        while (true) {
            if (rootAccess()) {
                try {
                    in.write(("id\n").getBytes("UTF-8"));
                    in.flush();
                    String s = new BufferedReader(new InputStreamReader(out)).readLine();
                    if (s.isEmpty() || !s.contains("uid=0")) {
                        in.close();
                        out.close();
                        process.destroy();
                        throw new IOException();
                    }
                } catch (IOException e) {
                    rootStatus = -1;
                    continue;
                }
                break;
            } else {
                // Try to gain non-root sh
                try {
                    process = Runtime.getRuntime().exec("sh");
                    in = new DataOutputStream(process.getOutputStream());
                    out = new DataInputStream(process.getInputStream());
                } catch (IOException e) {
                    // Nothing works....
                    shellProcess = null;
                    STDIN = null;
                    STDOUT = null;
                    isValid = false;
                    return;
                }
                break;
            }
        }

        isValid = true;
        shellProcess = process;
        STDIN = in;
        STDOUT = out;
        sh_raw("umask 022");
    }

    public static Shell getShell() {
        return new Shell();
    }

    public static Shell getShell(Context context) {
        MagiskManager magiskManager = Utils.getMagiskManager(context);
        if (!magiskManager.shell.isValid) {
            // Get new shell if needed
            magiskManager.shell = getShell();
        }
        return magiskManager.shell;
    }

    public static boolean rootAccess() {
        return rootStatus > 0;
    }

    public List<String> sh(String... commands) {
        List<String> res = new ArrayList<>();
        if (!isValid) return res;
        sh(res, commands);
        return res;
    }

    public void sh_raw(String... commands) {
        sh_raw(false, commands);
    }

    public void sh_raw(boolean stdout, String... commands) {
        if (!isValid) return;
        synchronized (shellProcess) {
            try {
                for (String command : commands) {
                    Logger.shell(command);
                    STDIN.write((command + (stdout ? "\n" : " >/dev/null\n")).getBytes("UTF-8"));
                    STDIN.flush();
                }
            } catch (IOException e) {
                e.printStackTrace();
                shellProcess.destroy();
                isValid = false;
            }
        }
    }

    public void sh(Collection<String> output, String... commands) {
        if (!isValid) return;
        try {
            shellProcess.exitValue();
            isValid = false;
            return;  // The process is dead, return
        } catch (IllegalThreadStateException ignored) {
            // This should be the expected result
        }
        synchronized (shellProcess) {
            StreamGobbler out = new StreamGobbler(STDOUT, output);
            out.start();
            sh_raw(true, commands);
            sh_raw(true, "echo \'-shell-done-\'");
            try { out.join(); } catch (InterruptedException ignored) {}
        }
    }

    public List<String> su(String... commands) {
        if (!rootAccess()) return sh();
        return sh(commands);
    }

    public void su_raw(String... commands) {
        if (!rootAccess()) return;
        sh_raw(commands);
    }

    public void su(Collection<String> output, String... commands) {
        if (!rootAccess()) return;
        sh(output, commands);
    }

    public static abstract class AbstractList<E> extends java.util.AbstractList<E> {

        @Override
        public abstract boolean add(E e);

        @Override
        public E get(int i) {
            return null;
        }

        @Override
        public int size() {
            return 0;
        }
    }
}
