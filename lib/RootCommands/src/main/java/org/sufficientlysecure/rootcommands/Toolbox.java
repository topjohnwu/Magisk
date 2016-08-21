/*
 * Copyright (C) 2012 Dominik Schürmann <dominik@dominikschuermann.de>
 * Copyright (c) 2012 Stephen Erickson, Chris Ravenscroft, Adam Shanks (RootTools)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.sufficientlysecure.rootcommands;

import android.os.StatFs;
import android.os.SystemClock;
import android.util.Log;

import org.sufficientlysecure.rootcommands.command.Command;
import org.sufficientlysecure.rootcommands.command.ExecutableCommand;
import org.sufficientlysecure.rootcommands.command.SimpleCommand;
import org.sufficientlysecure.rootcommands.util.BrokenBusyboxException;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.concurrent.TimeoutException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * All methods in this class are working with Androids toolbox. Toolbox is similar to busybox, but
 * normally shipped on every Android OS. You can find toolbox commands on
 * https://github.com/CyanogenMod/android_system_core/tree/ics/toolbox
 * <p>
 * This means that these commands are designed to work on every Android OS, with a _working_ toolbox
 * binary on it. They don't require busybox!
 */
public class Toolbox {
    public static final int REBOOT_HOTREBOOT = 1;
    public static final int REBOOT_REBOOT = 2;
    public static final int REBOOT_SHUTDOWN = 3;
    public static final int REBOOT_RECOVERY = 4;
    private Shell shell;

    /**
     * All methods in this class are working with Androids toolbox. Toolbox is similar to busybox,
     * but normally shipped on every Android OS.
     *
     * @param shell where to execute commands on
     */
    public Toolbox(Shell shell) {
        super();
        this.shell = shell;
    }

    /**
     * Checks if user accepted root access
     * <p>
     * (commands: id)
     *
     * @return true if user has given root access
     * @throws IOException
     * @throws TimeoutException
     * @throws BrokenBusyboxException
     */
    public boolean isRootAccessGiven() throws TimeoutException, IOException {
        SimpleCommand idCommand = new SimpleCommand("id");
        shell.add(idCommand).waitForFinish();

        return idCommand.getOutput().contains("uid=0");
    }

    /**
     * This method can be used to kill a running process
     * <p>
     * (commands: ps, kill)
     *
     * @param processName name of process to kill
     * @return <code>true</code> if process was found and killed successfully
     * @throws IOException
     * @throws TimeoutException
     * @throws BrokenBusyboxException
     */
    public boolean killAll(String processName) throws TimeoutException,
            IOException {
        Log.d(RootCommands.TAG, "Killing process " + processName);

        PsCommand psCommand = new PsCommand(processName);
        shell.add(psCommand).waitForFinish();

        // kill processes
        if (!psCommand.getPids().isEmpty()) {
            // example: kill -9 1234 1222 5343
            SimpleCommand killCommand = new SimpleCommand("kill -9 "
                    + psCommand.getPidsString());
            shell.add(killCommand).waitForFinish();

            return killCommand.getExitCode() == 0;
        } else {
            Log.d(RootCommands.TAG, "No pid found! Nothing was killed!");
            return false;
        }
    }

    /**
     * Kill a running executable
     * <p>
     * See README for more information how to use your own executables!
     *
     * @param executableName
     * @return
     * @throws BrokenBusyboxException
     * @throws TimeoutException
     * @throws IOException
     */
    public boolean killAllExecutable(String executableName) throws
            TimeoutException, IOException {
        return killAll(ExecutableCommand.EXECUTABLE_PREFIX + executableName + ExecutableCommand.EXECUTABLE_SUFFIX);
    }

    /**
     * This method can be used to to check if a process is running
     *
     * @param processName name of process to check
     * @return <code>true</code> if process was found
     * @throws IOException
     * @throws BrokenBusyboxException
     * @throws TimeoutException       (Could not determine if the process is running)
     */
    public boolean isProcessRunning(String processName) throws
            TimeoutException, IOException {
        PsCommand psCommand = new PsCommand(processName);
        shell.add(psCommand).waitForFinish();

        // if pids are available process is running!
        return !psCommand.getPids().isEmpty();
    }

    /**
     * Checks if binary is running
     *
     * @param binaryName
     * @return
     * @throws BrokenBusyboxException
     * @throws TimeoutException
     * @throws IOException
     */
    public boolean isBinaryRunning(String binaryName) throws
            TimeoutException, IOException {
        return isProcessRunning(ExecutableCommand.EXECUTABLE_PREFIX + binaryName
                + ExecutableCommand.EXECUTABLE_SUFFIX);
    }

    /**
     * @param file String that represent the file, including the full path to the file and its name.
     * @return File permissions as String, for example: 777, returns null on error
     * @throws IOException
     * @throws TimeoutException
     * @throws BrokenBusyboxException
     */
    public String getFilePermissions(String file) throws TimeoutException,
            IOException {
        Log.d(RootCommands.TAG, "Checking permissions for " + file);

        String permissions = null;

        if (fileExists(file)) {
            Log.d(RootCommands.TAG, file + " was found.");

            LsCommand lsCommand = new LsCommand(file);
            shell.add(lsCommand).waitForFinish();

            permissions = lsCommand.getPermissions();
        }

        return permissions;
    }

    /**
     * Sets permission of file
     *
     * @param file        absolute path to file
     * @param permissions String like 777
     * @return true if command worked
     * @throws BrokenBusyboxException
     * @throws TimeoutException
     * @throws IOException
     */
    public boolean setFilePermissions(String file, String permissions)
            throws TimeoutException, IOException {
        Log.d(RootCommands.TAG, "Set permissions of " + file + " to " + permissions);

        SimpleCommand chmodCommand = new SimpleCommand("chmod " + permissions + " " + file);
        shell.add(chmodCommand).waitForFinish();

        return chmodCommand.getExitCode() == 0;
    }

    /**
     * This will return a String that represent the symlink for a specified file.
     *
     * @param file The path to the file to get the Symlink for. (must have absolute path)
     * @return A String that represent the symlink for a specified file or null if no symlink
     * exists.
     * @throws IOException
     * @throws TimeoutException
     * @throws BrokenBusyboxException
     */
    public String getSymlink(String file) throws TimeoutException,
            IOException {
        Log.d(RootCommands.TAG, "Find symlink for " + file);

        String symlink = null;

        LsCommand lsCommand = new LsCommand(file);
        shell.add(lsCommand).waitForFinish();

        symlink = lsCommand.getSymlink();

        return symlink;
    }

    /**
     * Copys a file to a destination. Because cp is not available on all android devices, we use dd
     * or cat.
     *
     * @param source      example: /data/data/org.adaway/files/hosts
     * @param destination example: /system/etc/hosts
     * @param remountAsRw remounts the destination as read/write before writing to it
     * @return true if it was successfully copied
     * @throws BrokenBusyboxException
     * @throws IOException
     * @throws TimeoutException
     */
    public boolean copyFile(String source, String destination, boolean remountAsRw,
                            boolean preservePermissions) throws IOException,
            TimeoutException {

        /*
         * dd can only copy files, but we can not check if the source is a file without invoking
         * shell commands, because from Java we probably have no read access, thus we only check if
         * they are ending with trailing slashes
         */
        if (source.endsWith("/") || destination.endsWith("/")) {
            throw new FileNotFoundException("dd can only copy files!");
        }

        // remount destination as read/write before copying to it
        if (remountAsRw) {
            if (!remount(destination, "RW")) {
                Log.d(RootCommands.TAG,
                        "Remounting failed! There is probably no need to remount this partition!");
            }
        }

        // get permissions of source before overwriting
        String permissions = null;
        if (preservePermissions) {
            permissions = getFilePermissions(source);
        }

        boolean commandSuccess = false;

        SimpleCommand ddCommand = new SimpleCommand("dd if=" + source + " of="
                + destination);
        shell.add(ddCommand).waitForFinish();

        if (ddCommand.getExitCode() == 0) {
            commandSuccess = true;
        } else {
            // try cat if dd fails
            SimpleCommand catCommand = new SimpleCommand("cat " + source + " > "
                    + destination);
            shell.add(catCommand).waitForFinish();

            if (catCommand.getExitCode() == 0) {
                commandSuccess = true;
            }
        }

        // set back permissions from source to destination
        if (preservePermissions) {
            setFilePermissions(destination, permissions);
        }

        // remount destination back to read only
        if (remountAsRw) {
            if (!remount(destination, "RO")) {
                Log.d(RootCommands.TAG,
                        "Remounting failed! There is probably no need to remount this partition!");
            }
        }

        return commandSuccess;
    }

    /**
     * Shutdown or reboot device. Possible actions are REBOOT_HOTREBOOT, REBOOT_REBOOT,
     * REBOOT_SHUTDOWN, REBOOT_RECOVERY
     *
     * @throws IOException
     * @throws TimeoutException
     * @throws BrokenBusyboxException
     */
    public void reboot(int action) throws TimeoutException, IOException {
        if (action == REBOOT_HOTREBOOT) {
            killAll("system_server");
            // or: killAll("zygote");
        } else {
            String command;
            switch (action) {
                case REBOOT_REBOOT:
                    command = "reboot";
                    break;
                case REBOOT_SHUTDOWN:
                    command = "reboot -p";
                    break;
                case REBOOT_RECOVERY:
                    command = "reboot recovery";
                    break;
                default:
                    command = "reboot";
                    break;
            }

            SimpleCommand rebootCommand = new SimpleCommand(command);
            shell.add(rebootCommand).waitForFinish();

            if (rebootCommand.getExitCode() == -1) {
                Log.e(RootCommands.TAG, "Reboot failed!");
            }
        }
    }

    /**
     * Use this to check whether or not a file exists on the filesystem.
     *
     * @param file String that represent the file, including the full path to the file and its name.
     * @return a boolean that will indicate whether or not the file exists.
     * @throws IOException
     * @throws TimeoutException
     * @throws BrokenBusyboxException
     */
    public boolean fileExists(String file) throws TimeoutException,
            IOException {
        FileExistsCommand fileExistsCommand = new FileExistsCommand(file);
        shell.add(fileExistsCommand).waitForFinish();

        return fileExistsCommand.isFileExists();
    }

    /**
     * Execute user defined Java code while having temporary permissions on a file
     *
     * @param file
     * @param withPermissions
     * @throws BrokenBusyboxException
     * @throws TimeoutException
     * @throws IOException
     */
    public void withPermission(String file, String permission, WithPermissions withPermissions)
            throws TimeoutException, IOException {
        String oldPermissions = getFilePermissions(file);

        // set permissions (If set to 666, then Dalvik VM can also write to that file!)
        setFilePermissions(file, permission);

        // execute user defined code
        withPermissions.whileHavingPermissions();

        // set back to old permissions
        setFilePermissions(file, oldPermissions);
    }

    /**
     * Execute user defined Java code while having temporary write permissions on a file using chmod
     * 666
     *
     * @param file
     * @param withWritePermissions
     * @throws BrokenBusyboxException
     * @throws TimeoutException
     * @throws IOException
     */
    public void withWritePermissions(String file, WithPermissions withWritePermissions)
            throws TimeoutException, IOException {
        withPermission(file, "666", withWritePermissions);
    }

    /**
     * Sets system clock using /dev/alarm
     *
     * @param millis
     * @throws BrokenBusyboxException
     * @throws TimeoutException
     * @throws IOException
     */
    public void setSystemClock(final long millis) throws TimeoutException,
            IOException {
        withWritePermissions("/dev/alarm", new WithPermissions() {

            @Override
            void whileHavingPermissions() {
                SystemClock.setCurrentTimeMillis(millis);
            }
        });
    }

    /**
     * Adjust system clock by offset using /dev/alarm
     *
     * @param offset
     * @throws BrokenBusyboxException
     * @throws TimeoutException
     * @throws IOException
     */
    public void adjustSystemClock(final long offset) throws
            TimeoutException, IOException {
        withWritePermissions("/dev/alarm", new WithPermissions() {

            @Override
            void whileHavingPermissions() {
                SystemClock.setCurrentTimeMillis(System.currentTimeMillis() + offset);
            }
        });
    }

    /**
     * This will take a path, which can contain the file name as well, and attempt to remount the
     * underlying partition.
     * <p>
     * For example, passing in the following string:
     * "/system/bin/some/directory/that/really/would/never/exist" will result in /system ultimately
     * being remounted. However, keep in mind that the longer the path you supply, the more work
     * this has to do, and the slower it will run.
     *
     * @param file      file path
     * @param mountType mount type: pass in RO (Read only) or RW (Read Write)
     * @return a <code>boolean</code> which indicates whether or not the partition has been
     * remounted as specified.
     */
    public boolean remount(String file, String mountType) {
        // Recieved a request, get an instance of Remounter
        Remounter remounter = new Remounter(shell);
        // send the request
        return (remounter.remount(file, mountType));
    }

    /**
     * This will tell you how the specified mount is mounted. rw, ro, etc...
     *
     * @return <code>String</code> What the mount is mounted as.
     * @throws Exception if we cannot determine how the mount is mounted.
     */
    public String getMountedAs(String path) throws Exception {
        ArrayList<Mount> mounts = Remounter.getMounts();
        if (mounts != null) {
            for (Mount mount : mounts) {
                if (path.contains(mount.getMountPoint().getAbsolutePath())) {
                    Log.d(RootCommands.TAG, (String) mount.getFlags().toArray()[0]);
                    return (String) mount.getFlags().toArray()[0];
                }
            }

            throw new Exception();
        } else {
            throw new Exception();
        }
    }

    /**
     * Check if there is enough space on partition where target is located
     *
     * @param size   size of file to put on partition
     * @param target path where to put the file
     * @return true if it will fit on partition of target, false if it will not fit.
     */
    public boolean hasEnoughSpaceOnPartition(String target, long size) {
        try {
            // new File(target).getFreeSpace() (API 9) is not working on data partition

            // get directory without file
            String directory = new File(target).getParent().toString();

            StatFs stat = new StatFs(directory);
            long blockSize = stat.getBlockSize();
            long availableBlocks = stat.getAvailableBlocks();
            long availableSpace = availableBlocks * blockSize;

            Log.i(RootCommands.TAG, "Checking for enough space: Target: " + target
                    + ", directory: " + directory + " size: " + size + ", availableSpace: "
                    + availableSpace);

            if (size < availableSpace) {
                return true;
            } else {
                Log.e(RootCommands.TAG, "Not enough space on partition!");
                return false;
            }
        } catch (Exception e) {
            // if new StatFs(directory) fails catch IllegalArgumentException and just return true as
            // workaround
            Log.e(RootCommands.TAG, "Problem while getting available space on partition!", e);
            return true;
        }
    }

    /**
     * TODO: Not tested!
     *
     * @param toggle
     * @throws IOException
     * @throws TimeoutException
     * @throws BrokenBusyboxException
     */
    public void toggleAdbDaemon(boolean toggle) throws TimeoutException,
            IOException {
        SimpleCommand disableAdb = new SimpleCommand("setprop persist.service.adb.enable 0",
                "stop adbd");
        SimpleCommand enableAdb = new SimpleCommand("setprop persist.service.adb.enable 1",
                "stop adbd", "sleep 1", "start adbd");

        if (toggle) {
            shell.add(enableAdb).waitForFinish();
        } else {
            shell.add(disableAdb).waitForFinish();
        }
    }

    /**
     * This command class gets all pids to a given process name
     */
    private class PsCommand extends Command {
        private String processName;
        private ArrayList<String> pids;
        private String psRegex;
        private Pattern psPattern;

        public PsCommand(String processName) {
            super("ps");
            this.processName = processName;
            pids = new ArrayList<String>();

            /**
             * regex to get pid out of ps line, example:
             *
             * <pre>
             *  root    24736    1   12140  584   ffffffff 40010d14 S /data/data/org.adaway/files/blank_webserver
             * ^\\S \\s ([0-9]+)                          .*                                      processName    $
             * </pre>
             */
            psRegex = "^\\S+\\s+([0-9]+).*" + Pattern.quote(processName) + "$";
            psPattern = Pattern.compile(psRegex);
        }

        public ArrayList<String> getPids() {
            return pids;
        }

        public String getPidsString() {
            StringBuilder sb = new StringBuilder();
            for (String s : pids) {
                sb.append(s);
                sb.append(" ");
            }

            return sb.toString();
        }

        @Override
        public void output(int id, String line) {
            // general check if line contains processName
            if (line.contains(processName)) {
                Matcher psMatcher = psPattern.matcher(line);

                // try to match line exactly
                try {
                    if (psMatcher.find()) {
                        String pid = psMatcher.group(1);
                        // add to pids list
                        pids.add(pid);
                        Log.d(RootCommands.TAG, "Found pid: " + pid);
                    } else {
                        Log.d(RootCommands.TAG, "Matching in ps command failed!");
                    }
                } catch (Exception e) {
                    Log.e(RootCommands.TAG, "Error with regex!", e);
                }
            }
        }

        @Override
        public void afterExecution(int id, int exitCode) {
        }

    }

    /**
     * Ls command to get permissions or symlinks
     */
    private class LsCommand extends Command {
        private String fileName;
        private String permissionRegex;
        private Pattern permissionPattern;
        private String symlinkRegex;
        private Pattern symlinkPattern;

        private String symlink;
        private String permissions;

        public LsCommand(String file) {
            super("ls -l " + file);

            // get only filename:
            this.fileName = (new File(file)).getName();
            Log.d(RootCommands.TAG, "fileName: " + fileName);

            /**
             * regex to get pid out of ps line, example:
             *
             * <pre>
             * with busybox:
             *     lrwxrwxrwx     1 root root            15 Aug 13 12:14 dev/stdin -> /proc/self/fd/0
             *
             * with toolbox:
             *     lrwxrwxrwx root root            15 Aug 13 12:14 stdin -> /proc/self/fd/0
             *
             * Regex:
             * ^.*?(\\S{10})                     .*                                                  $
             * </pre>
             */
            permissionRegex = "^.*?(\\S{10}).*$";
            permissionPattern = Pattern.compile(permissionRegex);

            /**
             * regex to get symlink
             *
             * <pre>
             *     ->           /proc/self/fd/0
             * ^.*?\\-\\> \\s+  (.*)           $
             * </pre>
             */
            symlinkRegex = "^.*?\\-\\>\\s+(.*)$";
            symlinkPattern = Pattern.compile(symlinkRegex);
        }

        public String getSymlink() {
            return symlink;
        }

        public String getPermissions() {
            return permissions;
        }

        /**
         * Converts permission string from ls command to numerical value. Example: -rwxrwxrwx gets
         * to 777
         *
         * @param permissions
         * @return
         */
        private String convertPermissions(String permissions) {
            int owner = getGroupPermission(permissions.substring(1, 4));
            int group = getGroupPermission(permissions.substring(4, 7));
            int world = getGroupPermission(permissions.substring(7, 10));

            return "" + owner + group + world;
        }

        /**
         * Calculates permission for one group
         *
         * @param permission
         * @return value of permission string
         */
        private int getGroupPermission(String permission) {
            int value = 0;

            if (permission.charAt(0) == 'r') {
                value += 4;
            }
            if (permission.charAt(1) == 'w') {
                value += 2;
            }
            if (permission.charAt(2) == 'x') {
                value += 1;
            }

            return value;
        }

        @Override
        public void output(int id, String line) {
            // general check if line contains file
            if (line.contains(fileName)) {

                // try to match line exactly
                try {
                    Matcher permissionMatcher = permissionPattern.matcher(line);
                    if (permissionMatcher.find()) {
                        permissions = convertPermissions(permissionMatcher.group(1));

                        Log.d(RootCommands.TAG, "Found permissions: " + permissions);
                    } else {
                        Log.d(RootCommands.TAG, "Permissions were not found in ls command!");
                    }

                    // try to parse for symlink
                    Matcher symlinkMatcher = symlinkPattern.matcher(line);
                    if (symlinkMatcher.find()) {
                        /*
                         * TODO: If symlink points to a file in the same directory the path is not
                         * absolute!!!
                         */
                        symlink = symlinkMatcher.group(1);
                        Log.d(RootCommands.TAG, "Symlink found: " + symlink);
                    } else {
                        Log.d(RootCommands.TAG, "No symlink found!");
                    }
                } catch (Exception e) {
                    Log.e(RootCommands.TAG, "Error with regex!", e);
                }
            }
        }

        @Override
        public void afterExecution(int id, int exitCode) {
        }

    }

    /**
     * This command checks if a file exists
     */
    private class FileExistsCommand extends Command {
        private String file;
        private boolean fileExists = false;

        public FileExistsCommand(String file) {
            super("ls " + file);
            this.file = file;
        }

        public boolean isFileExists() {
            return fileExists;
        }

        @Override
        public void output(int id, String line) {
            if (line.trim().equals(file)) {
                fileExists = true;
            }
        }

        @Override
        public void afterExecution(int id, int exitCode) {
        }

    }

    public abstract class WithPermissions {
        abstract void whileHavingPermissions();
    }

}
