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

public class InstallMagisk extends ParallelTask<Void, Void, Boolean> {

    private static final int PATCH_MODE = 0;
    private static final int DIRECT_MODE = 1;

    private Uri mBootImg, mZip;
    private AdaptiveList<String> mList;
    private String mBootLocation;
    private boolean mKeepEnc, mKeepVerity;
    private int mode;

    private InstallMagisk(Activity context, AdaptiveList<String> list, Uri zip, boolean enc, boolean verity) {
        super(context);
        mList = list;
        mZip = zip;
        mKeepEnc = enc;
        mKeepVerity = verity;
    }

    public InstallMagisk(Activity context, AdaptiveList<String> list, Uri zip, boolean enc, boolean verity, Uri boot) {
        this(context, list, zip, enc, verity);
        mBootImg = boot;
        mode = PATCH_MODE;
    }

    public InstallMagisk(Activity context, AdaptiveList<String> list, Uri zip, boolean enc, boolean verity, String boot) {
        this(context, list, zip, enc, verity);
        mBootLocation = boot;
        mode = DIRECT_MODE;
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
        MagiskManager mm = getMagiskManager();
        if (mm == null) return false;

        File install = new File(mm.getApplicationInfo().dataDir, "install");
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
            try (InputStream in = mm.getContentResolver().openInputStream(mZip)) {
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

            File boot;
            switch (mode) {
                case PATCH_MODE:
                    boot = new File(install, "boot.img");
                    // Copy boot image to local
                    try (
                            InputStream in = mm.getContentResolver().openInputStream(mBootImg);
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
                    break;
                case DIRECT_MODE:
                    boot = new File(mBootLocation);
                    break;
                default:
                    return false;
            }

            mList.add("- Use boot image: " + boot);

            // Patch boot image
            getShell().sh(mList,
                    "cd " + install,
                    "KEEPFORCEENCRYPT=" + mKeepEnc + " KEEPVERITY=" + mKeepVerity + " sh " +
                            "update-binary indep boot_patch.sh " + boot + " 2>&1" +
                            " && echo 'Success!' || echo 'Failed!'"
            );

            if (!TextUtils.equals(mList.get(mList.size() - 1), "Success!"))
                return false;

            File patched_boot = new File(install, "new-boot.img");
            mList.add("");
            switch (mode) {
                case PATCH_MODE:
                    // Move boot image
                    File dest = new File(Environment.getExternalStorageDirectory() + "/MagiskManager/" + "patched_boot.img");
                    dest.getParentFile().mkdirs();
                    getShell().sh_raw("cp " + patched_boot + " " + dest);
                    mList.add("*********************************");
                    mList.add(" Patched Boot Image is placed in ");
                    mList.add(" " + dest + " ");
                    mList.add("*********************************");
                    break;
                case DIRECT_MODE:
                    // Direct flash boot image
                    getShell().su_raw("cat " + patched_boot + " /dev/zero | dd of=" + mBootLocation + " bs=4096");
                    mList.add("Flashing patched boot to " + mBootLocation);
                    break;
                default:
                    return false;
            }

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
        super.onPostExecute(result);
    }
}
