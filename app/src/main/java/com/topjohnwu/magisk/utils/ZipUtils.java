package com.topjohnwu.magisk.utils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

public class ZipUtils {

    public static void removeTopFolder(InputStream in, OutputStream out) {
        try {
            ZipInputStream source = new ZipInputStream(in);
            ZipOutputStream dest = new ZipOutputStream(out);
            ZipEntry entry;
            String path;
            int size;
            byte buffer[] = new byte[2048];
            while ((entry = source.getNextEntry()) != null) {
                // Remove the top directory from the path
                path = entry.toString().substring(entry.toString().indexOf("/") + 1);
                // If it's the top folder, ignore it
                if (path.isEmpty())
                    continue;
                // Don't include placeholder
                if (path.contains("system/placeholder"))
                    continue;
                dest.putNextEntry(new ZipEntry(path));
                while((size = source.read(buffer, 0, 2048)) != -1)
                    dest.write(buffer, 0, size);
            }
            source.close();
            dest.close();
        } catch (IOException e) {
            e.printStackTrace();
            Logger.dev("ZipUtils: removeTopFolder IO error!");
        }
    }
}
