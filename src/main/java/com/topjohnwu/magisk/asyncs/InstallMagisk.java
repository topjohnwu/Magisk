package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Build;
import android.text.TextUtils;
import android.view.View;

import com.topjohnwu.crypto.SignBoot;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.container.TarEntry;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ZipUtils;

import org.kamranzafar.jtar.TarInputStream;
import org.kamranzafar.jtar.TarOutputStream;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
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
    private List<String> console, logs;
    private String mBootLocation;
    private boolean mKeepEnc, mKeepVerity;
    private int mode;

    private InstallMagisk(Activity context, List<String> console, List<String> logs, Uri zip, boolean enc, boolean verity) {
        super(context);
        this.console = console;
        this.logs = logs;
        mZip = zip;
        mKeepEnc = enc;
        mKeepVerity = verity;
    }

    public InstallMagisk(Activity context, List<String> console, List<String> logs, Uri zip, boolean enc, boolean verity, Uri boot) {
        this(context, console, logs, zip, enc, verity);
        mBootImg = boot;
        mode = PATCH_MODE;
    }

    public InstallMagisk(Activity context, List<String> console, List<String> logs, Uri zip, boolean enc, boolean verity, String boot) {
        this(context, console, logs, zip, enc, verity);
        mBootLocation = boot;
        mode = DIRECT_MODE;
    }

    @Override
    protected Boolean doInBackground(Void... voids) {
        MagiskManager mm = MagiskManager.get();

        File install = new File(
                (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N ?
                        mm.createDeviceProtectedStorageContext() :
                        mm).getFilesDir().getParent()
                , "install");
        Shell.sh_raw("rm -rf " + install);

        List<String> abis = Arrays.asList(Build.SUPPORTED_ABIS);
        String arch;
        if (abis.contains("x86_64")) arch = "x64";
        else if (abis.contains("arm64-v8a")) arch = "arm64";
        else if (abis.contains("x86")) arch = "x86";
        else arch = "arm";
        console.add("- Device platform: " + arch);

        try {
            // Unzip files
            console.add("- Extracting files");
            try (InputStream in = mm.getContentResolver().openInputStream(mZip)) {
                if (in == null) throw new FileNotFoundException();
                BufferedInputStream buf = new BufferedInputStream(in);
                buf.mark(Integer.MAX_VALUE);
                ZipUtils.unzip(buf, install, arch + "/", true);
                buf.reset();
                ZipUtils.unzip(buf, install, "common/", true);
                buf.reset();
                ZipUtils.unzip(buf, install, "chromeos/", false);
                buf.reset();
                ZipUtils.unzip(buf, install, "META-INF/com/google/android/update-binary", true);
                buf.close();
            } catch (FileNotFoundException e) {
                console.add("! Invalid Uri");
                throw e;
            } catch (Exception e) {
                console.add("! Cannot unzip zip");
                throw e;
            }

            File boot = new File(install, "boot.img");
            switch (mode) {
                case PATCH_MODE:
                    console.add("- Use boot image: " + boot);
                    // Copy boot image to local
                    try (
                        InputStream in = mm.getContentResolver().openInputStream(mBootImg);
                        OutputStream out = new FileOutputStream(boot)
                    ) {
                        InputStream source;
                        if (in == null) throw new FileNotFoundException();

                        if (Utils.getNameFromUri(mm, mBootImg).endsWith(".tar")) {
                            // Extract boot.img from tar
                            TarInputStream tar = new TarInputStream(new BufferedInputStream(in));
                            org.kamranzafar.jtar.TarEntry entry;
                            while ((entry = tar.getNextEntry()) != null) {
                                if (entry.getName().equals("boot.img"))
                                    break;
                            }
                            source = tar;
                        } else {
                            // Direct copy raw image
                            source = new BufferedInputStream(in);
                        }
                        Utils.inToOut(source, out);
                    } catch (FileNotFoundException e) {
                        console.add("! Invalid Uri");
                        throw e;
                    } catch (IOException e) {
                        console.add("! Copy failed");
                        throw e;
                    }
                    break;
                case DIRECT_MODE:
                    console.add("- Use boot image: " + mBootLocation);
                    if (boot.createNewFile()) {
                        Shell.su("cat " + mBootLocation + " > " + boot);
                    } else {
                        console.add("! Dump boot image failed");
                        return false;
                    }
                    break;
                default:
                    return false;
            }

            boolean isSigned;
            try (InputStream in = new FileInputStream(boot)) {
                isSigned = SignBoot.verifySignature(in, null);
                if (isSigned) {
                    console.add("- Signed boot image detected");
                }
            } catch (Exception e) {
                console.add("! Unable to check signature");
                throw e;
            }

            // Force non-root shell
            Shell shell;
            if (Shell.rootAccess())
                shell = new Shell("sh");
            else
                shell = Shell.getShell();

            // Patch boot image
            shell.run(console, logs,
                    "cd " + install,
                    "KEEPFORCEENCRYPT=" + mKeepEnc + " KEEPVERITY=" + mKeepVerity + " sh " +
                            "update-binary indep boot_patch.sh " + boot + " || echo 'Failed!'");

            if (TextUtils.equals(console.get(console.size() - 1), "Failed!"))
                return false;

            shell.run(null, null,
                    "mv -f new-boot.img ../",
                    "mv bin/busybox busybox",
                    "rm -rf bin *.img update-binary",
                    "cd /");

            File patched_boot = new File(install.getParent(), "new-boot.img");

            if (isSigned) {
                console.add("- Signing boot image");
                File signed = new File(install.getParent(), "signed.img");
                AssetManager assets = mm.getAssets();
                try (
                        InputStream in = new FileInputStream(patched_boot);
                        OutputStream out = new BufferedOutputStream(new FileOutputStream(signed));
                        InputStream keyIn = assets.open(Const.PRIVATE_KEY_NAME);
                        InputStream certIn = assets.open(Const.PUBLIC_KEY_NAME)
                ) {
                    SignBoot.doSignature("/boot", in, out, keyIn, certIn);
                }
                shell.run_raw(false, false, "mv -f " + signed + " " + patched_boot);
            }

            switch (mode) {
                case PATCH_MODE:
                    File dest = new File(Const.EXTERNAL_PATH, "patched_boot" + mm.bootFormat);
                    dest.getParentFile().mkdirs();
                    OutputStream out;
                    switch (mm.bootFormat) {
                        case ".img.tar":
                            out = new TarOutputStream(new BufferedOutputStream(new FileOutputStream(dest)));
                            ((TarOutputStream) out).putNextEntry(new TarEntry(patched_boot, "boot.img"));
                            break;
                        default:
                        case ".img":
                            out = new BufferedOutputStream(new FileOutputStream(dest));
                            break;
                    }
                    try (InputStream in = new BufferedInputStream(new FileInputStream(patched_boot))) {
                        Utils.inToOut(in, out);
                        out.close();
                    }
                    console.add("");
                    console.add("*********************************");
                    console.add(" Patched Boot Image is placed in ");
                    console.add(" " + dest + " ");
                    console.add("*********************************");
                    break;
                case DIRECT_MODE:
                    // Direct flash boot image and patch dtbo if possible
                    Shell.getShell().run(console, logs,
                            "rm -rf /data/magisk/*",
                            "mkdir -p /data/magisk 2>/dev/null",
                            "mv -f " + install + "/* /data/magisk",
                            "rm -rf " + install,
                            "flash_boot_image " + patched_boot + " " + mBootLocation,
                            "patch_dtbo_image");
                    break;
                default:
                    return false;
            }

            patched_boot.delete();

            console.add("- All done!");
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    @Override
    protected void onPostExecute(Boolean result) {
        FlashActivity activity = (FlashActivity) getActivity();
        if (!result) {
            console.add("! Installation failed");
            activity.reboot.setVisibility(View.GONE);
        }
        activity.buttonPanel.setVisibility(View.VISIBLE);
    }
}
