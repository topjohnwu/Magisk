package com.topjohnwu.magisk.tasks;

import android.net.Uri;
import android.os.Build;
import android.text.TextUtils;

import androidx.annotation.MainThread;
import androidx.annotation.WorkerThread;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.container.TarEntry;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.net.DownloadProgressListener;
import com.topjohnwu.net.Networking;
import com.topjohnwu.signing.SignBoot;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.internal.NOPList;
import com.topjohnwu.superuser.internal.UiThreadHandler;
import com.topjohnwu.superuser.io.SuFile;
import com.topjohnwu.superuser.io.SuFileInputStream;
import com.topjohnwu.superuser.io.SuFileOutputStream;

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
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public abstract class MagiskInstaller {

    private List<String> console, logs;
    protected String srcBoot;
    protected File installDir;

    private class ProgressLog implements DownloadProgressListener {

        private int prev = -1;
        private int location;

        @Override
        public void onProgress(long bytesDownloaded, long totalBytes) {
            if (prev < 0) {
                location = console.size();
                console.add("... 0%");
            }
            int curr = (int) (100 * bytesDownloaded / totalBytes);
            if (prev != curr) {
                prev = curr;
                console.set(location, "... " + prev + "%");
            }
        }
    }

    protected MagiskInstaller() {
        console = NOPList.getInstance();
        logs = NOPList.getInstance();
    }

    public MagiskInstaller(List<String> out, List<String> err) {
        console = out;
        logs = err;
        installDir = new File(App.deContext.getFilesDir().getParent(), "install");
        Shell.sh("rm -rf " + installDir).exec();
        installDir.mkdirs();
    }

    protected boolean findImage() {
        console.add("- Detecting target image");
        srcBoot = ShellUtils.fastCmd("find_boot_image", "echo \"$BOOTIMAGE\"");
        if (srcBoot.isEmpty()) {
            console.add("! Unable to detect target image");
            return false;
        }
        return true;
    }

    protected boolean findSecondaryImage() {
        String slot = ShellUtils.fastCmd("echo $SLOT");
        String target = (TextUtils.equals(slot, "_a") ? "_b" : "_a");
        console.add("- Target slot: " + target);
        console.add("- Detecting target image");
        srcBoot = ShellUtils.fastCmd(
                "SLOT=" + target,
                "find_boot_image",
                "SLOT=" + slot,
                "echo \"$BOOTIMAGE\""
        );
        if (srcBoot.isEmpty()) {
            console.add("! Unable to detect target image");
            return false;
        }
        return true;
    }

    protected boolean extractZip() {
        String arch;
        if (Build.VERSION.SDK_INT >= 21) {
            List<String> abis = Arrays.asList(Build.SUPPORTED_ABIS);
            arch = abis.contains("x86") ? "x86" : "arm";
        } else {
            arch = TextUtils.equals(Build.CPU_ABI, "x86") ? "x86" : "arm";
        }

        console.add("- Device platform: " + Build.CPU_ABI);

        File zip = new File(App.self.getCacheDir(), "magisk.zip");

        if (!ShellUtils.checkSum("MD5", zip, Config.magiskMD5)) {
            console.add("- Downloading zip");
            Networking.get(Config.magiskLink)
                    .setDownloadProgressListener(new ProgressLog())
                    .execForFile(zip);
        } else {
            console.add("- Existing zip found");
        }

        try {
            ZipInputStream zi = new ZipInputStream(new BufferedInputStream(
                    new FileInputStream(zip), (int) zip.length()));
            ZipEntry ze;
            while ((ze = zi.getNextEntry()) != null) {
                if (ze.isDirectory())
                    continue;
                String name = null;
                String[] names = { arch + "/", "common/", "META-INF/com/google/android/update-binary" };
                for (String n : names) {
                    if (ze.getName().startsWith(n)) {
                        name = ze.getName().substring(ze.getName().lastIndexOf('/') + 1);
                        break;
                    }
                }
                if (name == null && ze.getName().startsWith("chromeos/"))
                    name = ze.getName();
                if (name == null)
                    continue;
                File dest = (installDir instanceof SuFile) ?
                        new SuFile(installDir, name) :
                        new File(installDir, name);
                dest.getParentFile().mkdirs();
                try (OutputStream out = new SuFileOutputStream(dest)) {
                    ShellUtils.pump(zi, out);
                }
            }
        } catch (IOException e) {
            console.add("! Cannot unzip zip");
            return false;
        }

        SuFile init64 = new SuFile(installDir, "magiskinit64");
        if (Build.VERSION.SDK_INT >= 21 && Build.SUPPORTED_64_BIT_ABIS.length != 0) {
            init64.renameTo(new SuFile(installDir, "magiskinit"));
        } else {
            init64.delete();
        }
        return true;
    }

    protected boolean copyBoot(Uri bootUri) {
        srcBoot = new File(installDir, "boot.img").getPath();
        console.add("- Copying image to cache");
        // Copy boot image to local
        try (InputStream in = App.self.getContentResolver().openInputStream(bootUri);
             OutputStream out = new FileOutputStream(srcBoot)) {
            if (in == null)
                throw new FileNotFoundException();

            InputStream src;
            if (Utils.getNameFromUri(App.self, bootUri).endsWith(".tar")) {
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

    protected boolean patchBoot() {
        boolean isSigned;
        try (InputStream in = new SuFileInputStream(srcBoot)) {
            isSigned = SignBoot.verifySignature(in, null);
            if (isSigned) {
                console.add("- Boot image is signed with AVB 1.0");
            }
        } catch (IOException e) {
            console.add("! Unable to check signature");
            return false;
        }

        if (!Shell.sh("cd " + installDir, Utils.fmt(
                "KEEPFORCEENCRYPT=%b KEEPVERITY=%b sh update-binary sh boot_patch.sh %s",
                Config.keepEnc, Config.keepVerity, srcBoot))
                .to(console, logs).exec().isSuccess())
            return false;

        Shell.Job job = Shell.sh("./magiskboot --cleanup",
                "mv bin/busybox busybox",
                "rm -rf magisk.apk bin boot.img update-binary",
                "cd /");

        File patched = new File(installDir, "new-boot.img");
        if (isSigned) {
            console.add("- Signing boot image with test keys");
            File signed = new File(installDir, "signed.img");
            try (InputStream in = new SuFileInputStream(patched);
                 OutputStream out = new BufferedOutputStream(new FileOutputStream(signed))) {
                SignBoot.doSignature("/boot", in, out, null, null);
            } catch (IOException e) {
                return false;
            }
            job.add("mv -f " + signed + " " + patched);
        }
        job.exec();
        return true;
    }

    protected boolean flashBoot() {
        if (!Shell.su(Utils.fmt("direct_install %s %s", installDir, srcBoot))
                .to(console, logs).exec().isSuccess())
            return false;
        if (!Config.keepVerity)
            Shell.su("patch_dtbo_image").to(console, logs).exec();
        return true;
    }

    protected boolean storeBoot() {
        File patched = new File(installDir, "new-boot.img");
        String fmt = Config.get(Config.Key.BOOT_FORMAT);
        File dest = new File(Const.EXTERNAL_PATH, "patched_boot" + fmt);
        dest.getParentFile().mkdirs();
        OutputStream os;
        try {
            switch (fmt) {
                case ".img.tar":
                    os = new TarOutputStream(new BufferedOutputStream(new FileOutputStream(dest)));
                    ((TarOutputStream) os).putNextEntry(new TarEntry(patched, "boot.img"));
                    break;
                default:
                case ".img":
                    os = new BufferedOutputStream(new FileOutputStream(dest));
                    break;
            }
            try (InputStream in = new SuFileInputStream(patched)) {
                ShellUtils.pump(in, os);
                os.close();
            }
        } catch (IOException e) {
            console.add("! Failed to store boot to " + dest);
            return false;
        }
        Shell.sh("rm -f " + patched).exec();
        console.add("");
        console.add("****************************");
        console.add(" Patched image is placed in ");
        console.add(" " + dest + " ");
        console.add("****************************");
        return true;
    }

    protected boolean postOTA() {
        SuFile bootctl = new SuFile("/data/adb/bootctl");
        try (InputStream in = Networking.get(Const.Url.BOOTCTL_URL).execForInputStream().getResult();
             OutputStream out = new SuFileOutputStream(bootctl)) {
            ShellUtils.pump(in, out);
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
        Shell.su("post_ota " + bootctl.getParent()).exec();
        console.add("***************************************");
        console.add(" Next reboot will boot to second slot!");
        console.add("***************************************");
        return true;
    }

    @WorkerThread
    protected abstract boolean operations();

    @MainThread
    protected abstract void onResult(boolean success);

    public void exec() {
        App.THREAD_POOL.execute(() -> {
            boolean b = operations();
            UiThreadHandler.run(() -> onResult(b));
        });
    }

}
