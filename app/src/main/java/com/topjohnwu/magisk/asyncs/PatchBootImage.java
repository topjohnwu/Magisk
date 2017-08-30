package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.text.TextUtils;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.AdaptiveList;
import com.topjohnwu.magisk.utils.ZipUtils;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.List;

public class PatchBootImage extends ParallelTask<Void, Void, Boolean> {

    private Uri mBootImg, mZip;
    private AdaptiveList<String> mList;
    private File dest;

    public PatchBootImage(Activity context, Uri zip, Uri boot, AdaptiveList<String> list) {
        super(context);
        mBootImg = boot;
        mList = list;
        mZip = zip;
        dest = new File(Environment.getExternalStorageDirectory() + "/MagiskManager/" + "patched_boot.img");
    }

    @Override
    protected void onPreExecute() {
        // UI updates must run in the UI thread
        mList.setCallback(this::publishProgress);
    }

    @Override
    protected void onProgressUpdate(Void... values) {
        mList.updateView();
    }

    @Override
    protected Boolean doInBackground(Void... voids) {
        MagiskManager magiskManager = getMagiskManager();
        if (magiskManager == null) return false;

        File install = new File(magiskManager.getApplicationInfo().dataDir, "install");
        getShell().sh_raw("rm -rf " + install);

        List<String> abis = Arrays.asList(Build.SUPPORTED_ABIS);
        String arch;
        if (abis.contains("x86_64")) arch = "x64";
        else if (abis.contains("arm64-v8a")) arch = "arm64";
        else if (abis.contains("x86")) arch = "x86";
        else arch = "arm";
        mList.add("- Device platform: " + arch);

        try {
            // Unzip files
            mList.add("- Extracting files");
            try (InputStream in = magiskManager.getContentResolver().openInputStream(mZip)) {
                if (in == null) throw new FileNotFoundException();
                BufferedInputStream buf = new BufferedInputStream(in);
                buf.mark(Integer.MAX_VALUE);
                ZipUtils.unzip(buf, install, arch, true);
                buf.reset();
                ZipUtils.unzip(buf, install, "common", true);
                buf.reset();
                ZipUtils.unzip(buf, install, "chromeos", false);
                buf.reset();
                ZipUtils.unzip(buf, install, "META-INF/com/google/android/update-binary", true);
            } catch (FileNotFoundException e) {
                mList.add("! Invalid Uri");
                throw e;
            } catch (Exception e) {
                mList.add("! Cannot unzip zip");
                throw e;
            }

            // Copy boot image
            File boot = new File(install, "boot.img");
            try (
                InputStream in = magiskManager.getContentResolver().openInputStream(mBootImg);
                OutputStream out = new FileOutputStream(boot);
            ) {
                if (in == null) throw new FileNotFoundException();
                byte buffer[] = new byte[1024];
                int length;
                while ((length = in.read(buffer)) > 0)
                    out.write(buffer, 0, length);
            } catch (FileNotFoundException e) {
                mList.add("! Invalid Uri");
                throw e;
            } catch (IOException e) {
                mList.add("! Copy failed");
                throw e;
            }

            mList.add("- Use boot image: " + boot);

            // Patch boot image
            getShell().sh(mList,
                    "chmod -R 755 " + install,
                    "cd " + install,
                    "sh update-binary indep boot_patch.sh " + boot +
                            " && echo 'Success!' || echo 'Failed!'"
            );

            if (!TextUtils.equals(mList.get(mList.size() - 1), "Success!"))
                return false;

            // Move boot image
            File source = new File(install, "new-boot.img");
            dest.getParentFile().mkdirs();
            getShell().sh_raw("mv " + source + " " + dest);

            // Finals
            getShell().sh_raw(
                    "mv bin/busybox busybox",
                    "rm -rf bin *.img update-binary");
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    @Override
    protected void onPostExecute(Boolean result) {
        if (result) {
            mList.add("");
            mList.add("*********************************");
            mList.add(" Patched Boot Image is placed in ");
            mList.add(" " + dest + " ");
            mList.add("*********************************");
        }
        super.onPostExecute(result);
    }
}
