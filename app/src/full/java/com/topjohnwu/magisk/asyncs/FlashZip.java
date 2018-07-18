package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.net.Uri;
import android.text.TextUtils;
import android.view.View;

import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.components.SnackbarMaker;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ZipUtils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;

public class FlashZip extends ParallelTask<Void, Void, Integer> {

    private Uri mUri;
    private File mCachedFile;
    private List<String> console, logs;

    public FlashZip(Activity context, Uri uri, List<String> console, List<String> logs) {
        super(context);
        mUri = uri;
        this.console = console;
        this.logs = logs;
        mCachedFile = new File(context.getCacheDir(), "install.zip");
    }

    private boolean unzipAndCheck() throws Exception {
        ZipUtils.unzip(mCachedFile, mCachedFile.getParentFile(), "META-INF/com/google/android", true);
        return ShellUtils.fastCmdResult("grep -q '#MAGISK' " + new File(mCachedFile.getParentFile(), "updater-script"));
    }

    @Override
    protected Integer doInBackground(Void... voids) {
        MagiskManager mm = MagiskManager.get();
        try {
            console.add("- Copying zip to temp directory");

            mCachedFile.delete();
            try (
                InputStream in = mm.getContentResolver().openInputStream(mUri);
                OutputStream out = new BufferedOutputStream(new FileOutputStream(mCachedFile))
            ) {
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
            if (!unzipAndCheck()) return 0;
            console.add("- Installing " + Utils.getNameFromUri(mm, mUri));
            Shell.Sync.su(console, logs,
                    "cd " + mCachedFile.getParent(),
                    "BOOTMODE=true sh update-binary dummy 1 " + mCachedFile + " || echo 'Failed!'"
            );

            if (TextUtils.equals(console.get(console.size() - 1), "Failed!"))
                return -1;

        } catch (Exception e) {
            e.printStackTrace();
            return -1;
        }
        console.add("- All done!");
        return 1;
    }

    // -1 = error, manual install; 0 = invalid zip; 1 = success
    @Override
    protected void onPostExecute(Integer result) {
        FlashActivity activity = (FlashActivity) getActivity();
        Shell.Async.su(
                "rm -rf " + mCachedFile.getParent(),
                "rm -rf " + Const.TMP_FOLDER_PATH
        );
        switch (result) {
            case -1:
                console.add("! Installation failed");
                SnackbarMaker.showUri(getActivity(), mUri);
                break;
            case 0:
                console.add("! This zip is not a Magisk Module!");
                break;
            case 1:
                // Success
                new LoadModules().exec();
                break;
        }
        activity.reboot.setVisibility(result > 0 ? View.VISIBLE : View.GONE);
        activity.buttonPanel.setVisibility(View.VISIBLE);
    }
}
