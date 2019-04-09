package com.topjohnwu.magisk.tasks;

import android.net.Uri;
import android.os.Build;
import android.text.TextUtils;

import androidx.annotation.MainThread;
import androidx.annotation.WorkerThread;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
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

import org.kamranzafar.jtar.TarEntry;
import org.kamranzafar.jtar.TarHeader;
import org.kamranzafar.jtar.TarInputStream;
import org.kamranzafar.jtar.TarOutputStream;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public abstract class MagiskInstaller {

    protected String srcBoot;
    protected File destFile;
    protected File installDir;

    private List<String> console, logs;
    private boolean isTar = false;

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
        srcBoot = ShellUtils.fastCmd("find_boot_image", "echo \"$BOOTIMAGE\"");
        if (srcBoot.isEmpty()) {
            console.add("! Unable to detect target image");
            return false;
        }
        console.add("- Target image: " + srcBoot);
        return true;
    }

    protected boolean findSecondaryImage() {
        String slot = ShellUtils.fastCmd("echo $SLOT");
        String target = (TextUtils.equals(slot, "_a") ? "_b" : "_a");
        console.add("- Target slot: " + target);
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
        console.add("- Target image: " + srcBoot);
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

        File init64 = SuFile.open(installDir, "magiskinit64");
        if (Build.VERSION.SDK_INT >= 21 && Build.SUPPORTED_64_BIT_ABIS.length != 0) {
            init64.renameTo(SuFile.open(installDir, "magiskinit"));
        } else {
            init64.delete();
        }
        Shell.sh("cd " + installDir, "chmod 755 *").exec();
        return true;
    }

    private TarEntry newEntry(String name, long size) {
        console.add("-- Writing: " + name);
        return new TarEntry(TarHeader.createHeader(name, size, 0, false, 0644));
    }

    private void handleTar(InputStream in) throws IOException {
        console.add("- Processing tar file");
        boolean vbmeta = false;
        try (TarInputStream tarIn = new TarInputStream(in);
             TarOutputStream tarOut = new TarOutputStream(destFile)) {
            TarEntry entry;
            while ((entry = tarIn.getNextEntry()) != null) {
                if (entry.getName().contains("boot.img")
                        || entry.getName().contains("recovery.img")) {
                    String name = entry.getName();
                    console.add("-- Extracting: " + name);
                    File extract = new File(installDir, name);
                    try (FileOutputStream fout = new FileOutputStream(extract)) {
                        ShellUtils.pump(tarIn, fout);
                    }
                    if (name.contains(".lz4")) {
                        console.add("-- Decompressing: " + name);
                        Shell.sh("./magiskboot --decompress " + extract).to(console).exec();
                    }
                } else if (entry.getName().contains("vbmeta.img")) {
                    vbmeta = true;
                    ByteBuffer buf = ByteBuffer.allocate(256);
                    buf.put("AVB0".getBytes()); // magic
                    buf.putInt(1);              // required_libavb_version_major
                    buf.putInt(120, 2);         // flags
                    buf.position(128);          // release_string
                    buf.put("avbtool 1.1.0".getBytes());
                    tarOut.putNextEntry(newEntry("vbmeta.img", 256));
                    tarOut.write(buf.array());
                } else {
                    console.add("-- Writing: " + entry.getName());
                    tarOut.putNextEntry(entry);
                    ShellUtils.pump(tarIn, tarOut);
                }
            }
            File boot = SuFile.open(installDir, "boot.img");
            File recovery = SuFile.open(installDir, "recovery.img");
            if (vbmeta && recovery.exists() && boot.exists()) {
                // Install Magisk to recovery
                srcBoot = recovery.getPath();
                // Repack boot image to prevent restore
                Shell.sh(
                        "./magiskboot --unpack boot.img",
                        "./magiskboot --repack boot.img",
                        "./magiskboot --cleanup",
                        "mv new-boot.img boot.img").exec();
                try (InputStream sin = new SuFileInputStream(boot)) {
                    tarOut.putNextEntry(newEntry("boot.img", boot.length()));
                    ShellUtils.pump(sin, tarOut);
                }
                boot.delete();
            } else {
                if (!boot.exists()) {
                    console.add("! No boot image found");
                    throw new IOException();
                }
                srcBoot = boot.getPath();
            }
        }
    }

    protected boolean handleFile(Uri uri) {
        try (InputStream in = new BufferedInputStream(App.self.getContentResolver().openInputStream(uri))) {
            in.mark(500);
            byte[] magic = new byte[5];
            if (in.skip(257) != 257 || in.read(magic) != magic.length) {
                console.add("! Invalid file");
                return false;
            }
            in.reset();
            if (Arrays.equals(magic, "ustar".getBytes())) {
                isTar = true;
                destFile = new File(Const.EXTERNAL_PATH, "magisk_patched.tar");
                handleTar(in);
            } else {
                // Raw image
                srcBoot = new File(installDir, "boot.img").getPath();
                destFile = new File(Const.EXTERNAL_PATH, "magisk_patched.img");
                console.add("- Copying image to cache");
                try (OutputStream out = new FileOutputStream(srcBoot)) {
                    ShellUtils.pump(in, out);
                }
            }
        } catch (IOException e) {
            console.add("! Process error");
            e.printStackTrace();
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

        if (!Shell.sh(Utils.fmt(
                "KEEPFORCEENCRYPT=%b KEEPVERITY=%b RECOVERYMODE=%b " +
                "sh update-binary sh boot_patch.sh %s",
                Config.keepEnc, Config.keepVerity, Config.recovery, srcBoot))
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
        File patched = SuFile.open(installDir, "new-boot.img");
        try {
            OutputStream os;
            if (isTar) {
                os = new TarOutputStream(destFile, true);
                ((TarOutputStream) os).putNextEntry(newEntry(
                                srcBoot.contains("recovery") ? "recovery.img" : "boot.img",
                                patched.length()));
            } else {
                os = new BufferedOutputStream(new FileOutputStream(destFile));
            }
            try (InputStream in = new SuFileInputStream(patched);
                OutputStream out = os) {
                ShellUtils.pump(in, out);
            }
        } catch (IOException e) {
            console.add("! Failed to output to " + destFile);
            e.printStackTrace();
        }
        patched.delete();
        console.add("");
        console.add("****************************");
        console.add(" Output file is placed in ");
        console.add(" " + destFile + " ");
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
