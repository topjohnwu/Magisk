package com.topjohnwu.magisk.core.utils;

import android.os.Build;

import java.nio.file.attribute.FileTime;
import java.util.zip.ZipEntry;

public class Desugar {
    public static FileTime getLastModifiedTime(ZipEntry entry) {
        if (Build.VERSION.SDK_INT >= 26) {
            return entry.getLastModifiedTime();
        } else {
            return FileTime.fromMillis(entry.getTime());
        }
    }

    public static FileTime getLastAccessTime(ZipEntry entry) {
        if (Build.VERSION.SDK_INT >= 26) {
            return entry.getLastAccessTime();
        } else {
            return null;
        }
    }

    public static FileTime getCreationTime(ZipEntry entry) {
        if (Build.VERSION.SDK_INT >= 26) {
            return entry.getCreationTime();
        } else {
            return null;
        }
    }
}
