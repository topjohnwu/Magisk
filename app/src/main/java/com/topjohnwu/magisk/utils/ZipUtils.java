package com.topjohnwu.magisk.utils;

import com.topjohnwu.signing.JarMap;
import com.topjohnwu.signing.SignAPK;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.io.SuFile;
import com.topjohnwu.superuser.io.SuFileOutputStream;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class ZipUtils {

    public static void unzip(File zip, File folder, String path, boolean junkPath) throws IOException {
        InputStream in = new BufferedInputStream(new FileInputStream(zip));
        unzip(in, folder, path, junkPath);
        in.close();
    }

    public static void unzip(InputStream zip, File folder, String path, boolean junkPath) throws IOException {
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
                if (!dest.getParentFile().exists() && !dest.getParentFile().mkdirs()) {
                    dest = new SuFile(folder, name);
                    dest.getParentFile().mkdirs();
                }
                try (OutputStream out = new SuFileOutputStream(dest)) {
                    ShellUtils.pump(zipfile, out);
                }
            }
        }
        catch(IOException e) {
            e.printStackTrace();
            throw e;
        }
    }

    public static void signZip(File input, File output) throws Exception {
        try (JarMap map = new JarMap(input, false);
             BufferedOutputStream out = new BufferedOutputStream(new FileOutputStream(output))) {
            SignAPK.sign(map, out);
        }
    }
}
