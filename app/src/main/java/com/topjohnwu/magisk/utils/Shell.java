package com.topjohnwu.magisk.utils;

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

    // Run with the same shell by default
    public static List<String> su(String... commands) {
        return su(false, commands);
    }

    public static List<String> su(boolean newShell, String... commands) {
        List<String> res;
        Process process;
        DataOutputStream STDIN;
        StreamGobbler STDOUT;

        if (newShell) {
            res = Collections.synchronizedList(new ArrayList<String>());
            try {
                process = Runtime.getRuntime().exec(sh("getprop magisk.supath").get(0) + "/su");
                STDIN = new DataOutputStream(process.getOutputStream());
                STDOUT = new StreamGobbler(process.getInputStream(), res);
            } catch (IOException e) {
                try {
                    // Improper root
                    process = Runtime.getRuntime().exec("su");
                    STDIN = new DataOutputStream(process.getOutputStream());
                    STDOUT = new StreamGobbler(process.getInputStream(), res);
                } catch (IOException err) {
                    return null;
                }
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
                        try { STDOUT.join(100); } catch (InterruptedException err) { return null; }
                    }
                }
            }
        } catch (IOException e) {
            if (!e.getMessage().contains("EPIPE")) {
                Logger.dev("Shell: Root shell error...");
                return null;
            }
        } catch(InterruptedException e) {
            Logger.dev("Shell: Root shell error...");
            return null;
        }

        return new ArrayList<>(res);
    }
}
