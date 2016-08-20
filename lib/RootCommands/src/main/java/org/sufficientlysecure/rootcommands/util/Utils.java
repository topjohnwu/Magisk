/*
 * Copyright (C) 2012 Dominik Schürmann <dominik@dominikschuermann.de>
 * Copyright (c) 2012 Michael Elsdörfer (Android Autostarts)
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

package org.sufficientlysecure.rootcommands.util;

import org.sufficientlysecure.rootcommands.RootCommands;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Map;

public class Utils {
    /*
     * The emulator and ADP1 device both have a su binary in /system/xbin/su, but it doesn't allow
     * apps to use it (su app_29 $ su su: uid 10029 not allowed to su).
     * 
     * Cyanogen used to have su in /system/bin/su, in newer versions it's a symlink to
     * /system/xbin/su.
     * 
     * The Archos tablet has it in /data/bin/su, since they don't have write access to /system yet.
     */
    static final String[] BinaryPlaces = {"/data/bin/", "/system/bin/", "/system/xbin/", "/sbin/",
            "/data/local/xbin/", "/data/local/bin/", "/system/sd/xbin/", "/system/bin/failsafe/",
            "/data/local/"};

    /**
     * Determine the path of the su executable.
     * <p>
     * Code from https://github.com/miracle2k/android-autostarts, use under Apache License was
     * agreed by Michael Elsdörfer
     */
    public static String getSuPath() {
        for (String p : BinaryPlaces) {
            File su = new File(p + "su");
            if (su.exists()) {
                Log.d(RootCommands.TAG, "su found at: " + p);
                return su.getAbsolutePath();
            } else {
                Log.v(RootCommands.TAG, "No su in: " + p);
            }
        }
        Log.d(RootCommands.TAG, "No su found in a well-known location, " + "will just use \"su\".");
        return "su";
    }

    /**
     * This code is adapted from java.lang.ProcessBuilder.start().
     * <p>
     * The problem is that Android doesn't allow us to modify the map returned by
     * ProcessBuilder.environment(), even though the docstring indicates that it should. This is
     * because it simply returns the SystemEnvironment object that System.getenv() gives us. The
     * relevant portion in the source code is marked as "// android changed", so presumably it's not
     * the case in the original version of the Apache Harmony project.
     * <p>
     * Note that simply passing the environment variables we want to Process.exec won't be good
     * enough, since that would override the environment we inherited completely.
     * <p>
     * We needed to be able to set a CLASSPATH environment variable for our new process in order to
     * use the "app_process" command directly. Note: "app_process" takes arguments passed on to the
     * Dalvik VM as well; this might be an alternative way to set the class path.
     * <p>
     * Code from https://github.com/miracle2k/android-autostarts, use under Apache License was
     * agreed by Michael Elsdörfer
     */
    public static Process runWithEnv(String command, ArrayList<String> customAddedEnv,
                                     String baseDirectory) throws IOException {

        Map<String, String> environment = System.getenv();
        String[] envArray = new String[environment.size()
                + (customAddedEnv != null ? customAddedEnv.size() : 0)];
        int i = 0;
        for (Map.Entry<String, String> entry : environment.entrySet()) {
            envArray[i++] = entry.getKey() + "=" + entry.getValue();
        }
        if (customAddedEnv != null) {
            for (String entry : customAddedEnv) {
                envArray[i++] = entry;
            }
        }

        Process process;
        if (baseDirectory == null) {
            process = Runtime.getRuntime().exec(command, envArray, null);
        } else {
            process = Runtime.getRuntime().exec(command, envArray, new File(baseDirectory));
        }
        return process;
    }
}
