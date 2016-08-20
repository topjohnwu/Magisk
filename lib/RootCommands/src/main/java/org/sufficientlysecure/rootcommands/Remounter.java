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

import org.sufficientlysecure.rootcommands.command.SimpleCommand;
import org.sufficientlysecure.rootcommands.util.Log;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.LineNumberReader;
import java.util.ArrayList;
import java.util.Locale;

//no modifier, this means it is package-private. Only our internal classes can use this.
class Remounter {

    private Shell shell;

    public Remounter(Shell shell) {
        super();
        this.shell = shell;
    }

    /**
     * This will return an ArrayList of the class Mount. The class mount contains the following
     * property's: device mountPoint type flags
     * <p>
     * These will provide you with any information you need to work with the mount points.
     *
     * @return <code>ArrayList<Mount></code> an ArrayList of the class Mount.
     * @throws Exception if we cannot return the mount points.
     */
    protected static ArrayList<Mount> getMounts() throws Exception {

        final String tempFile = "/data/local/RootToolsMounts";

        // copy /proc/mounts to tempfile. Directly reading it does not work on 4.3
        Shell shell = Shell.startRootShell();
        Toolbox tb = new Toolbox(shell);
        tb.copyFile("/proc/mounts", tempFile, false, false);
        tb.setFilePermissions(tempFile, "777");
        shell.close();

        LineNumberReader lnr = null;
        lnr = new LineNumberReader(new FileReader(tempFile));
        String line;
        ArrayList<Mount> mounts = new ArrayList<Mount>();
        while ((line = lnr.readLine()) != null) {

            Log.d(RootCommands.TAG, line);

            String[] fields = line.split(" ");
            mounts.add(new Mount(new File(fields[0]), // device
                    new File(fields[1]), // mountPoint
                    fields[2], // fstype
                    fields[3] // flags
            ));
        }
        lnr.close();

        return mounts;
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
    protected boolean remount(String file, String mountType) {

        // if the path has a trailing slash get rid of it.
        if (file.endsWith("/") && !file.equals("/")) {
            file = file.substring(0, file.lastIndexOf("/"));
        }
        // Make sure that what we are trying to remount is in the mount list.
        boolean foundMount = false;
        while (!foundMount) {
            try {
                for (Mount mount : getMounts()) {
                    Log.d(RootCommands.TAG, mount.getMountPoint().toString());

                    if (file.equals(mount.getMountPoint().toString())) {
                        foundMount = true;
                        break;
                    }
                }
            } catch (Exception e) {
                Log.e(RootCommands.TAG, "Exception", e);
                return false;
            }
            if (!foundMount) {
                try {
                    file = (new File(file).getParent()).toString();
                } catch (Exception e) {
                    Log.e(RootCommands.TAG, "Exception", e);
                    return false;
                }
            }
        }
        Mount mountPoint = findMountPointRecursive(file);

        Log.d(RootCommands.TAG, "Remounting " + mountPoint.getMountPoint().getAbsolutePath()
                + " as " + mountType.toLowerCase(Locale.US));
        final boolean isMountMode = mountPoint.getFlags().contains(mountType.toLowerCase(Locale.US));

        if (!isMountMode) {
            // grab an instance of the internal class
            try {
                SimpleCommand command = new SimpleCommand("busybox mount -o remount,"
                        + mountType.toLowerCase(Locale.US) + " " + mountPoint.getDevice().getAbsolutePath()
                        + " " + mountPoint.getMountPoint().getAbsolutePath(),
                        "toolbox mount -o remount," + mountType.toLowerCase(Locale.US) + " "
                                + mountPoint.getDevice().getAbsolutePath() + " "
                                + mountPoint.getMountPoint().getAbsolutePath(), "mount -o remount,"
                        + mountType.toLowerCase(Locale.US) + " "
                        + mountPoint.getDevice().getAbsolutePath() + " "
                        + mountPoint.getMountPoint().getAbsolutePath(),
                        "/system/bin/toolbox mount -o remount," + mountType.toLowerCase(Locale.US) + " "
                                + mountPoint.getDevice().getAbsolutePath() + " "
                                + mountPoint.getMountPoint().getAbsolutePath());

                // execute on shell
                shell.add(command).waitForFinish();

            } catch (Exception e) {
            }

            mountPoint = findMountPointRecursive(file);
        }

        if (mountPoint != null) {
            Log.d(RootCommands.TAG, mountPoint.getFlags() + " AND " + mountType.toLowerCase(Locale.US));
            if (mountPoint.getFlags().contains(mountType.toLowerCase(Locale.US))) {
                Log.d(RootCommands.TAG, mountPoint.getFlags().toString());
                return true;
            } else {
                Log.d(RootCommands.TAG, mountPoint.getFlags().toString());
            }
        } else {
            Log.d(RootCommands.TAG, "mountPoint is null");
        }
        return false;
    }

    private Mount findMountPointRecursive(String file) {
        try {
            ArrayList<Mount> mounts = getMounts();
            for (File path = new File(file); path != null; ) {
                for (Mount mount : mounts) {
                    if (mount.getMountPoint().equals(path)) {
                        return mount;
                    }
                }
            }
            return null;
        } catch (IOException e) {
            throw new RuntimeException(e);
        } catch (Exception e) {
            Log.e(RootCommands.TAG, "Exception", e);
        }
        return null;
    }
}
