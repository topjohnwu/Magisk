package com.topjohnwu.magisk.utils;

import android.text.TextUtils;

import com.topjohnwu.magisk.MagiskManager;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * Modified by topjohnwu, based on Chainfire's libsuperuser
 */

public class Shell {

    // -2 = not initialized; -1 = no shell; 0 = non root shell; 1 = root shell
    public static int status = -2;

    private final Process process;
    private final OutputStream STDIN;
    private final InputStream STDOUT;
    private final InputStream STDERR;

    private static void testRootShell(Shell shell) throws IOException {
        shell.STDIN.write(("id\n").getBytes("UTF-8"));
        shell.STDIN.flush();
        String s = new BufferedReader(new InputStreamReader(shell.STDOUT)).readLine();
        if (TextUtils.isEmpty(s) || !s.contains("uid=0")) {
            shell.STDIN.close();
            shell.STDIN.close();
            throw new IOException();
        }
    }

    public Shell(String command) throws IOException {
        process = Runtime.getRuntime().exec(command);
        STDIN = process.getOutputStream();
        STDOUT = process.getInputStream();
        STDERR = process.getErrorStream();
    }

    public static Shell getShell() {
        MagiskManager mm = MagiskManager.get();
        boolean needNewShell = mm.shell == null;

        if (!needNewShell) {
            try {
                mm.shell.process.exitValue();
                // The process is dead
                needNewShell = true;
            } catch (IllegalThreadStateException ignored) {
                // This should be the expected result
            }
        }

        if (needNewShell) {
            status = 1;
            try {
                mm.shell = new Shell("su --mount-master");
                testRootShell(mm.shell);
            } catch (IOException e) {
                // Mount master not implemented
                try {
                    mm.shell = new Shell("su");
                    testRootShell(mm.shell);
                } catch (IOException e1) {
                    // No root exists
                    status = 0;
                    try {
                        mm.shell = new Shell("sh");
                    } catch (IOException e2) {
                        status = -1;
                        return null;
                    }
                }
            }
            if (rootAccess()) {
                // Load utility shell scripts
                try (InputStream in  = mm.getAssets().open(Const.UTIL_FUNCTIONS)) {
                    mm.shell.loadInputStream(in);
                } catch (IOException e) {
                    e.printStackTrace();
                }

                // Root shell initialization
                String bbpath = Const.BUSYBOX_PATH();
                mm.shell.run_raw(false, false,
                        "export PATH=" + bbpath + ":$PATH",
                        "mount_partitions",
                        "find_boot_image",
                        "migrate_boot_backup");
            }
        }

        return mm.shell;
    }

    public static boolean rootAccess() {
        if (status == -2) getShell();
        return status > 0;
    }

    public void run(Collection<String> output, Collection<String> error, String... commands) {
        StreamGobbler out, err;
        synchronized (process) {
            try {
                out = new StreamGobbler(STDOUT, output);
                err = new StreamGobbler(STDERR, error);
                out.start();
                err.start();
                run_raw(output != null, error != null, commands);
                STDIN.write("echo \'-shell-done-\'\necho \'-shell-done-\' >&2\n".getBytes("UTF-8"));
                STDIN.flush();
                try {
                    out.join();
                    err.join();
                } catch (InterruptedException ignored) {}
            } catch (IOException e) {
                e.printStackTrace();
                process.destroy();
            }
        }
    }

    public void run_raw(boolean stdout, boolean stderr, String... commands) {
        String suffix = "\n";
        if (!stderr) suffix = " 2>/dev/null" + suffix;
        if (!stdout) suffix = " >/dev/null" + suffix;
        synchronized (process) {
            try {
                for (String command : commands) {
                    Logger.shell(true, command);
                    STDIN.write((command + suffix).getBytes("UTF-8"));
                    STDIN.flush();
                }
            } catch (IOException e) {
                e.printStackTrace();
                process.destroy();
            }
        }
    }

    public void loadInputStream(InputStream in) {
        synchronized (process) {
            try {
                Utils.inToOut(in, STDIN);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static List<String> sh(String... commands) {
        List<String> res = new ArrayList<>();
        sh(res, commands);
        return res;
    }

    public static void sh(Collection<String> output, String... commands) {
        Shell shell = getShell();
        if (shell == null)
            return;
        shell.run(output, null, commands);
    }

    public static void sh_raw(String... commands) {
        Shell shell = getShell();
        if (shell == null)
            return;
        shell.run_raw(false, false, commands);
    }

    public static List<String> su(String... commands) {
        if (!rootAccess()) return sh();
        return sh(commands);
    }

    public static void su(Collection<String> output, String... commands) {
        if (!rootAccess()) return;
        sh(output, commands);
    }

    public static void su_raw(String... commands) {
        if (!rootAccess()) return;
        sh_raw(commands);
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
