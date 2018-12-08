package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.app.ProgressDialog;
import android.net.Uri;
import android.os.Build;
import android.text.TextUtils;
import android.view.View;
import android.widget.Toast;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.container.TarEntry;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;
import com.topjohnwu.magisk.utils.ZipUtils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.internal.NOPList;
import com.topjohnwu.superuser.io.SuFile;
import com.topjohnwu.superuser.io.SuFileInputStream;
import com.topjohnwu.superuser.io.SuFileOutputStream;
import com.topjohnwu.utils.SignBoot;

import org.kamranzafar.jtar.TarInputStream;
import org.kamranzafar.jtar.TarOutputStream;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.util.Arrays;
import java.util.List;

import androidx.annotation.NonNull;

public class InstallMagisk extends ParallelTask<Void, Void, Boolean> {

    private static final int PATCH_MODE = 0;
    public static final int DIRECT_MODE = 1;
    private static final int FIX_ENV_MODE = 2;
    public static final int SECOND_SLOT_MODE = 3;

    private Uri bootUri;
    private List<String> console, logs;
    private String mBoot;
    private int mode;
    private File installDir;
    private ProgressDialog dialog;
    private MagiskManager mm;

    public InstallMagisk(Activity context) {
        super(context);
        mm = Data.MM();
        mode = FIX_ENV_MODE;
    }

    public InstallMagisk(Activity context, List<String> console, List<String> logs, int mode) {
        this(context);
        this.console = console;
        this.logs = logs;
        this.mode = mode;
    }

    public InstallMagisk(FlashActivity context, List<String> console, List<String> logs, Uri boot) {
        this(context, console, logs, PATCH_MODE);
        bootUri = boot;
    }

    @Override
    protected void onPreExecute() {
        if (mode == FIX_ENV_MODE) {
            Activity a = getActivity();
            dialog = ProgressDialog.show(a, a.getString(R.string.setup_title), a.getString(R.string.setup_msg));
            console = NOPList.getInstance();
        }
    }

    private class ProgressStream extends FilterInputStream {

        private int prev = -1;
        private int progress = 0;
        private int total;

        private ProgressStream(HttpURLConnection conn) throws IOException {
            super(conn.getInputStream());
            total = conn.getContentLength();
            console.add("... 0%");
        }

        private void update(int step) {
            progress += step;
            int curr = (int) (100 * (double) progress / total);
            if (prev != curr) {
                prev = curr;
                console.set(console.size() - 1, "... " + prev + "%");
            }
        }

        @Override
        public int read() throws IOException {
            int b = super.read();
            if (b > 0)
                update(1);
            return b;
        }

        @Override
        public int read(@NonNull byte[] b) throws IOException {
            return read(b, 0, b.length);
        }

        @Override
        public int read(@NonNull byte[] b, int off, int len) throws IOException {
            int step = super.read(b, off, len);
            if (step > 0)
                update(step);
            return step;
        }
    }

    private void extractFiles(String arch) throws IOException {
        File zip = new File(mm.getFilesDir(), "magisk.zip");
        BufferedInputStream buf;

        if (!ShellUtils.checkSum("MD5", zip, Data.magiskMD5)) {
            console.add("- Downloading zip");
            HttpURLConnection conn = WebService.mustRequest(Data.magiskLink);
            buf = new BufferedInputStream(new ProgressStream(conn), conn.getContentLength());
            buf.mark(conn.getContentLength() + 1);
            try (OutputStream out = new FileOutputStream(zip)) {
                ShellUtils.pump(buf, out);
            }
            buf.reset();
            conn.disconnect();
        } else {
            console.add("- Existing zip found");
            buf = new BufferedInputStream(new FileInputStream(zip), (int) zip.length());
            buf.mark((int) zip.length() + 1);
        }

        console.add("- Extracting files");
        try (InputStream in = buf) {
            ZipUtils.unzip(in, installDir, arch + "/", true);
            in.reset();
            ZipUtils.unzip(in, installDir, "common/", true);
            in.reset();
            ZipUtils.unzip(in, installDir, "chromeos/", false);
            in.reset();
            ZipUtils.unzip(in, installDir, "META-INF/com/google/android/update-binary", true);
        } catch (IOException e) {
            console.add("! Cannot unzip zip");
            throw e;
        }
        Shell.sh(Utils.fmt("chmod -R 755 %s/*; %s/magiskinit -x magisk %s/magisk",
                installDir, installDir, installDir)).exec();
    }

    private boolean dumpBoot() {
        console.add("- Copying image to cache");
        // Copy boot image to local
        try (InputStream in = mm.getContentResolver().openInputStream(bootUri);
             OutputStream out = new FileOutputStream(mBoot)
        ) {
            if (in == null)
                throw new FileNotFoundException();

            InputStream src;
            if (Utils.getNameFromUri(mm, bootUri).endsWith(".tar")) {
                // Extract boot.img from tar
                TarInputStream tar = new TarInputStream(new BufferedInputStream(in));
                org.kamranzafar.jtar.TarEntry entry;
                while ((entry = tar.getNextEntry()) != null) {
                    if (entry.getName().equals("boot.img"))
                        break;
                }
                src = tar;
            } else {
                // Direct copy raw image
                src = new BufferedInputStream(in);
            }
            ShellUtils.pump(src, out);
        } catch (FileNotFoundException e) {
            console.add("! Invalid Uri");
            return false;
        } catch (IOException e) {
            console.add("! Copy failed");
            return false;
        }
        return true;
    }

    private File patchBoot() throws IOException {
        boolean isSigned;
        try (InputStream in = new SuFileInputStream(mBoot)) {
            isSigned = SignBoot.verifySignature(in, null);
            if (isSigned) {
                console.add("- Boot image is signed with AVB 1.0");
            }
        } catch (IOException e) {
            console.add("! Unable to check signature");
            throw e;
        }

        // Patch boot image
        if (!Shell.sh("cd " + installDir, Utils.fmt(
                "KEEPFORCEENCRYPT=%b KEEPVERITY=%b sh update-binary indep boot_patch.sh %s",
                Data.keepEnc, Data.keepVerity, mBoot))
                .to(console, logs).exec().isSuccess())
            return null;

        Shell.Job job = Shell.sh("mv bin/busybox busybox",
                "rm -rf magisk.apk bin boot.img update-binary",
                "cd /");

        File patched = new File(installDir, "new-boot.img");
        if (isSigned) {
            console.add("- Signing boot image with test keys");
            File signed = new File(installDir, "signed.img");
            try (InputStream in = new SuFileInputStream(patched);
                 OutputStream out = new BufferedOutputStream(new FileOutputStream(signed))
            ) {
                SignBoot.doSignature("/boot", in, out, null, null);
            }
            job.add("mv -f " + signed + " " + patched);
        }
        job.exec();
        return patched;
    }

    private boolean outputBoot(File patched) throws IOException {
        switch (mode) {
            case PATCH_MODE:
                String fmt = mm.prefs.getString(Const.Key.BOOT_FORMAT, ".img");
                File dest = new File(Const.EXTERNAL_PATH, "patched_boot" + fmt);
                dest.getParentFile().mkdirs();
                OutputStream out;
                switch (fmt) {
                    case ".img.tar":
                        out = new TarOutputStream(new BufferedOutputStream(new FileOutputStream(dest)));
                        ((TarOutputStream) out).putNextEntry(new TarEntry(patched, "boot.img"));
                        break;
                    default:
                    case ".img":
                        out = new BufferedOutputStream(new FileOutputStream(dest));
                        break;
                }
                try (InputStream in = new SuFileInputStream(patched)) {
                    ShellUtils.pump(in, out);
                    out.close();
                }
                Shell.sh("rm -f " + patched).exec();
                console.add("");
                console.add("****************************");
                console.add(" Patched image is placed in ");
                console.add(" " + dest + " ");
                console.add("****************************");
                break;
            case SECOND_SLOT_MODE:
            case DIRECT_MODE:
                if (!Shell.su(Utils.fmt("direct_install %s %s", installDir, mBoot))
                        .to(console, logs).exec().isSuccess())
                    return false;
                if (!Data.keepVerity)
                    Shell.su("find_dtbo_image", "patch_dtbo_image").to(console, logs).exec();
                break;
        }
        return true;
    }

    private void postOTA() {
        SuFile bootctl = new SuFile(Const.MAGISK_PATH + "/.core/bootctl");
        try (InputStream in = mm.getResources().openRawResource(R.raw.bootctl);
             OutputStream out = new SuFileOutputStream(bootctl)) {
            ShellUtils.pump(in, out);
            Shell.su("post_ota " + bootctl.getParent()).exec();
            console.add("***************************************");
            console.add(" Next reboot will boot to second slot!");
            console.add("***************************************");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    protected Boolean doInBackground(Void... voids) {
        if (mode == FIX_ENV_MODE) {
            installDir = new File("/data/adb/magisk");
            Shell.su("rm -rf /data/adb/magisk/*").exec();
        } else {
            installDir = new File(
                    (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N ?
                            mm.createDeviceProtectedStorageContext() : mm)
                            .getFilesDir().getParent()
                    , "install");
            Shell.sh("rm -rf " + installDir).exec();
            installDir.mkdirs();
        }

        switch (mode) {
            case PATCH_MODE:
                mBoot = new File(installDir, "boot.img").getAbsolutePath();
                if (!dumpBoot())
                    return false;
                break;
            case DIRECT_MODE:
                console.add("- Detecting target image");
                mBoot = ShellUtils.fastCmd("find_boot_image", "echo \"$BOOTIMAGE\"");
                break;
            case SECOND_SLOT_MODE:
                String slot = ShellUtils.fastCmd("echo $SLOT");
                String target = (TextUtils.equals(slot, "_a") ? "_b" : "_a");
                console.add("- Target slot: " + target);
                console.add("- Detecting target image");
                mBoot = ShellUtils.fastCmd(
                        "SLOT=" + target,
                        "find_boot_image",
                        "SLOT=" + slot,
                        "echo \"$BOOTIMAGE\""
                );
                break;
            case FIX_ENV_MODE:
                mBoot = "";
                break;
        }
        if (mBoot == null) {
            console.add("! Unable to detect target image");
            return false;
        }

        if (mode == DIRECT_MODE || mode == SECOND_SLOT_MODE)
            console.add("- Target image: " + mBoot);

        List<String> abis = Arrays.asList(Build.SUPPORTED_ABIS);
        String arch = abis.contains("x86") ? "x86" : "arm";

        console.add("- Device platform: " + Build.SUPPORTED_ABIS[0]);

        try {
            extractFiles(arch);
            if (mode == FIX_ENV_MODE) {
                Shell.su("fix_env").exec();
            } else {
                File patched = patchBoot();
                if (patched == null)
                    return false;
                if (!outputBoot(patched))
                    return false;
                if (mode == SECOND_SLOT_MODE)
                    postOTA();
                console.add("- All done!");
            }
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
        return true;
    }

    @Override
    protected void onPostExecute(Boolean result) {
        if (mode == FIX_ENV_MODE) {
            dialog.dismiss();
            Utils.toast(result ? R.string.setup_done : R.string.setup_fail, Toast.LENGTH_LONG);
        } else {
            // Running in FlashActivity
            FlashActivity activity = (FlashActivity) getActivity();
            if (!result) {
                Shell.sh("rm -rf " + installDir).submit();
                console.add("! Installation failed");
                activity.reboot.setVisibility(View.GONE);
            }
            activity.buttonPanel.setVisibility(View.VISIBLE);
        }
    }
}
