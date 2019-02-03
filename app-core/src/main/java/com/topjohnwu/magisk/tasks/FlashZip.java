package com.topjohnwu.magisk.tasks;

import android.net.Uri;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ZipUtils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.internal.UiThreadHandler;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;

public abstract class FlashZip {

    private Uri mUri;
    private File tmpFile;
    private List<String> console, logs;

    public FlashZip(Uri uri, List<String> out, List<String> err) {
        mUri = uri;
        console = out;
        logs = err;
        tmpFile = new File(App.self.getCacheDir(), "install.zip");
    }

    private boolean unzipAndCheck() throws IOException {
        ZipUtils.unzip(tmpFile, tmpFile.getParentFile(), "META-INF/com/google/android", true);
        return Shell.su("grep -q '#MAGISK' " + new File(tmpFile.getParentFile(), "updater-script"))
                .exec().isSuccess();
    }

    private boolean flash() throws IOException {
        console.add("- Copying zip to temp directory");
        try (InputStream in = App.self.getContentResolver().openInputStream(mUri);
             OutputStream out = new BufferedOutputStream(new FileOutputStream(tmpFile))) {
            if (in == null) throw new FileNotFoundException();
            InputStream buf= new BufferedInputStream(in);
            ShellUtils.pump(buf, out);
        } catch (FileNotFoundException e) {
            console.add("! Invalid Uri");
            throw e;
        } catch (IOException e) {
            console.add("! Cannot copy to cache");
            throw e;
        }
        try {
            if (!unzipAndCheck()) {
                console.add("! This zip is not a Magisk Module!");
                return false;
            }
        } catch (IOException e) {
            console.add("! Unzip error");
            throw e;
        }
        console.add("- Installing " + Utils.getNameFromUri(App.self, mUri));
        return Shell.su("cd " + tmpFile.getParent(),
                "BOOTMODE=true sh update-binary dummy 1 " + tmpFile)
                .to(console, logs)
                .exec().isSuccess();
    }

    public void exec() {
        App.THREAD_POOL.execute(() -> {
            boolean success = false;
            try {
                success = flash();
            } catch (IOException ignored) {}
            Shell.su("cd /", "rm -rf " + tmpFile.getParent() + " " + Const.TMP_FOLDER_PATH).submit();
            boolean finalSuccess = success;
            UiThreadHandler.run(() -> onResult(finalSuccess));
        });
    }

    protected abstract void onResult(boolean success);
}
