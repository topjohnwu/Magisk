package com.topjohnwu.magisk.utils;

import android.content.res.AssetManager;

import com.topjohnwu.crypto.JarMap;
import com.topjohnwu.crypto.SignAPK;
import com.topjohnwu.magisk.MagiskManager;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class ZipUtils {

    static {
        System.loadLibrary("zipadjust");
    }

    public native static void zipAdjust(String filenameIn, String filenameOut);

    public static void unzip(File zip, File folder, String path, boolean junkPath) throws Exception {
        InputStream in = new BufferedInputStream(new FileInputStream(zip));
        unzip(in, folder, path, junkPath);
        in.close();
    }

    public static void unzip(InputStream zip, File folder, String path, boolean junkPath) throws Exception {
        try {
            ZipInputStream zipfile = new ZipInputStream(zip);
            ZipEntry entry;
            while ((entry = zipfile.getNextEntry()) != null) {
                if (!entry.getName().startsWith(path) || entry.isDirectory()){
                    // Ignore directories, only create files
                    continue;
                }
                String name;
                if (junkPath) {
                    name = entry.getName().substring(entry.getName().lastIndexOf('/') + 1);
                } else {
                    name = entry.getName();
                }
                File dest = new File(folder, name);
                dest.getParentFile().mkdirs();
                try (FileOutputStream out = new FileOutputStream(dest)) {
                    Utils.inToOut(zipfile, out);
                }
            }
        } catch(Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    public static void signZip(InputStream is, File output, boolean minSign) throws Exception {
        try (JarMap map = new JarMap(is, false)) {
            signZip(map, output, minSign);
        }
    }

    public static void signZip(File input, File output, boolean minSign) throws Exception {
        try (JarMap map = new JarMap(input, false)) {
            signZip(map, output, minSign);
        }
    }

    public static void signZip(JarMap input, File output, boolean minSign) throws Exception {
        try (BufferedOutputStream out = new BufferedOutputStream(new FileOutputStream(output))) {
            signZip(input, out, minSign);
        }
    }

    public static void signZip(JarMap input, OutputStream output, boolean minSign) throws Exception {
        SignAPK.signZip(null, null, input, output, minSign);
    }
}
