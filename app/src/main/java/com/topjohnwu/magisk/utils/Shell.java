package com.topjohnwu.magisk.utils;

import com.topjohnwu.magisk.asyncs.RootTask;

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
    public static final Object lock = new Object();

    private static boolean isInit = false;
    private static Process rootShell;
    private static DataOutputStream rootSTDIN;
    private static StreamGobbler rootSTDOUT;
    private static List<String> rootOutList = Collections.synchronizedList(new ArrayList<String>());

    public static void init() {

        isInit = true;

        try {
            rootShell = Runtime.getRuntime().exec("su");
            rootStatus = 1;
        } catch (IOException err) {
            // No root
            rootStatus = 0;
            return;
        }

        rootSTDIN = new DataOutputStream(rootShell.getOutputStream());
        rootSTDOUT = new StreamGobbler(rootShell.getInputStream(), rootOutList, true);
        rootSTDOUT.start();

        // Setup umask and PATH
        su("umask 022");

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
        return isInit && rootStatus > 0;
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

    // Run with the same shell by default
    public static List<String> su(String... commands) {
        return su(false, commands);
    }

    public static List<String> su(boolean newShell, String... commands) {
        List<String> res;
        Process process;
        DataOutputStream STDIN;
        StreamGobbler STDOUT;

        // Create the default shell if not init
        if (!newShell && !isInit) {
            init();
        }

        if (!newShell && !rootAccess()) {
            return null;
        }

        if (newShell) {
            res = Collections.synchronizedList(new ArrayList<String>());
            try {
                process = Runtime.getRuntime().exec("su");
                STDIN = new DataOutputStream(process.getOutputStream());
                STDOUT = new StreamGobbler(process.getInputStream(), res);

                // Run the new shell with busybox and proper umask
                STDIN.write(("umask 022\n").getBytes("UTF-8"));
                STDIN.flush();
            } catch (IOException err) {
                return null;
            }
            STDOUT.start();
        } else {
            process = rootShell;
            STDIN = rootSTDIN;
            STDOUT = rootSTDOUT;
            res = rootOutList;
            res.clear();
        }

        try {
            for (String write : commands) {
                STDIN.write((write + "\n").getBytes("UTF-8"));
                STDIN.flush();
                Logger.shell(true, write);
            }
            if (newShell) {
                STDIN.write("exit\n".getBytes("UTF-8"));
                STDIN.flush();
                process.waitFor();

                try {
                    STDIN.close();
                } catch (IOException ignore) {
                    // might be closed already
                }

                STDOUT.join();
                process.destroy();
            } else {
                STDIN.write(("echo\n").getBytes("UTF-8"));
                STDIN.flush();
                STDIN.write(("echo \'-root-done-\'\n").getBytes("UTF-8"));
                STDIN.flush();
                while (true) {
                    try {
                        // Process terminated, it means the interactive shell has some issues
                        process.exitValue();
                        rootStatus = -1;
                        return null;
                    } catch (IllegalThreadStateException e) {
                        // Process still running, gobble output until done
                        int end = res.size() - 1;
                        if (end > 0) {
                            if (res.get(end).equals("-root-done-")) {
                                res.remove(end);
                                if (res.get(end -1).isEmpty()) {
                                    res.remove(end -1);
                                }
                                break;
                            }
                        }
                        try { STDOUT.join(100); } catch (InterruptedException err) {
                            rootStatus = -1;
                            return null;
                        }
                    }
                }
            }
        } catch (IOException e) {
            if (!e.getMessage().contains("EPIPE")) {
                Logger.dev("Shell: Root shell error...");
                rootStatus = -1;
                return null;
            }
        } catch(InterruptedException e) {
            Logger.dev("Shell: Root shell error...");
            rootStatus = -1;
            return null;
        }

        return new ArrayList<>(res);
    }

    public static void su_async(List<String> result, String... commands) {
        new RootTask<Void, Void, Void>() {
            @Override
            protected Void doInRoot(Void... params) {
                List<String> ret = Shell.su(commands);
                if (result != null) result.addAll(ret);
                return null;
            }
        }.exec();
    }
}
