package com.topjohnwu.magisk.obfuscate;

import android.content.res.Resources;

import com.topjohnwu.magisk.R;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

public class Dump {

    public static void resAPK(Resources res, File apk) {
        try (InputStream is = res.openRawResource(R.raw.arsc);
             ZipOutputStream zout = new ZipOutputStream(new FileOutputStream(apk))) {
            zout.putNextEntry(new ZipEntry("resources.arsc"));
            byte[] buf = new byte[4096];
            for (int read; (read = is.read(buf)) >= 0;) {
                zout.write(buf, 0, read);
            }
        } catch (IOException e) {
            // Should not happen
            e.printStackTrace();
        }
    }
}
