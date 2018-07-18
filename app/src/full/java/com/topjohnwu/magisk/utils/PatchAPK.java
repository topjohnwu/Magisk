package com.topjohnwu.magisk.utils;

import com.topjohnwu.utils.JarMap;

import java.io.OutputStream;
import java.util.jar.JarEntry;

public class PatchAPK {

    private static int findOffset(byte buf[], byte pattern[]) {
        int offset = -1;
        for (int i = 0; i < buf.length - pattern.length; ++i) {
            boolean match = true;
            for (int j = 0; j < pattern.length; ++j) {
                if (buf[i + j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                offset = i;
                break;
            }
        }
        return offset;
    }

    /* It seems that AAPT sometimes generate another type of string format */
    private static boolean fallbackPatch(byte xml[], String from, String to) {

        byte[] target = new byte[from.length() * 2 + 2];
        for (int i = 0; i < from.length(); ++i) {
            target[i * 2] = (byte) from.charAt(i);
        }
        int offset = findOffset(xml, target);
        if (offset < 0)
            return false;
        byte[] dest = new byte[target.length - 2];
        for (int i = 0; i < to.length(); ++i) {
            dest[i * 2] = (byte) to.charAt(i);
        }
        System.arraycopy(dest, 0, xml, offset, dest.length);
        return true;
    }

    private static boolean findAndPatch(byte xml[], String from, String to) {
        byte target[] = (from + '\0').getBytes();
        int offset = findOffset(xml, target);
        if (offset < 0)
            return fallbackPatch(xml, from, to);
        System.arraycopy(to.getBytes(), 0, xml, offset, to.length());
        return true;
    }

    public static boolean patchPackageID(String fileName, OutputStream out, String from, String to) {
        try {
            JarMap apk = new JarMap(fileName);
            JarEntry je = apk.getJarEntry(Const.ANDROID_MANIFEST);
            byte xml[] = apk.getRawData(je);

            if (!findAndPatch(xml, from, to))
                return false;
            if (!findAndPatch(xml, from + ".provider", to + ".provider"))
                return false;

            // Write in changes
            apk.getOutputStream(je).write(xml);

            // Sign the APK
            ZipUtils.signZip(apk, out);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }
}
