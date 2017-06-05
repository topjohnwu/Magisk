package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.app.ProgressDialog;
import android.net.Uri;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.components.AlertDialogBuilder;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ZipUtils;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;

public class FlashZip extends RootTask<Void, String, Integer> {

    private Uri mUri;
    private File mCachedFile, mScriptFile, mCheckFile;

    private String mFilename;
    private ProgressDialog progress;

    public FlashZip(Activity context, Uri uri) {
        super(context);
        mUri = uri;

        mCachedFile = new File(magiskManager.getCacheDir(), "install.zip");
        mScriptFile = new File(magiskManager.getCacheDir(), "/META-INF/com/google/android/update-binary");
        mCheckFile = new File(mScriptFile.getParent(), "updater-script");

        // Try to get the filename ourselves
        mFilename = Utils.getNameFromUri(magiskManager, mUri);
    }

    private void copyToCache() throws Throwable {
        publishProgress(magiskManager.getString(R.string.copying_msg));

        if (mCachedFile.exists() && !mCachedFile.delete()) {
            Logger.error("FlashZip: Error while deleting already existing file");
            throw new IOException();
        }
        try (
                InputStream in = magiskManager.getContentResolver().openInputStream(mUri);
                OutputStream outputStream = new FileOutputStream(mCachedFile)
        ) {
            byte buffer[] = new byte[1024];
            int length;
            if (in == null) throw new FileNotFoundException();
            while ((length = in.read(buffer)) > 0)
                outputStream.write(buffer, 0, length);

            Logger.dev("FlashZip: File created successfully - " + mCachedFile.getPath());
        } catch (FileNotFoundException e) {
            Logger.error("FlashZip: Invalid Uri");
            throw e;
        } catch (IOException e) {
            Logger.error("FlashZip: Error in creating file");
            throw e;
        }
    }

    private boolean unzipAndCheck() throws Exception {
        ZipUtils.unzip(mCachedFile, mCachedFile.getParentFile(), "META-INF/com/google/android");
        List<String> ret;
        ret = Utils.readFile(mCheckFile.getPath());
        return Utils.isValidShellResponse(ret) && ret.get(0).contains("#MAGISK");
    }

    private int cleanup(int ret) {
        Shell.su(
                "rm -rf " + mCachedFile.getParent() + "/*",
                "rm -rf " + MagiskManager.TMP_FOLDER_PATH
        );
        return ret;
    }

    @Override
    protected void onPreExecute() {
        progress = new ProgressDialog(activity);
        progress.setTitle(R.string.zip_install_progress_title);
        progress.show();
    }

    @Override
    protected void onProgressUpdate(String... values) {
        progress.setMessage(values[0]);
    }

    @Override
    protected Integer doInRoot(Void... voids) {
        Logger.dev("FlashZip Running... " + mFilename);
        List<String> ret;
        try {
            copyToCache();
            if (!unzipAndCheck()) return cleanup(0);
            publishProgress(magiskManager.getString(R.string.zip_install_progress_msg, mFilename));
            ret = Shell.su(
                    "BOOTMODE=true sh " + mScriptFile + " dummy 1 " + mCachedFile,
                    "if [ $? -eq 0 ]; then echo true; else echo false; fi"
            );
            if (!Utils.isValidShellResponse(ret)) return -1;
            Logger.dev("FlashZip: Console log:");
            for (String line : ret) {
                Logger.dev(line);
            }
            if (Boolean.parseBoolean(ret.get(ret.size() - 1)))
                return cleanup(1);

        } catch (Throwable e) {
            e.printStackTrace();
        }
        return cleanup(-1);
    }

    // -1 = error, manual install; 0 = invalid zip; 1 = success
    @Override
    protected void onPostExecute(Integer result) {
        progress.dismiss();
        switch (result) {
            case -1:
                Toast.makeText(magiskManager, magiskManager.getString(R.string.install_error), Toast.LENGTH_LONG).show();
                Utils.showUriSnack(activity, mUri);
                break;
            case 0:
                Toast.makeText(magiskManager, magiskManager.getString(R.string.invalid_zip), Toast.LENGTH_LONG).show();
                break;
            case 1:
                onSuccess();
                break;
        }
        super.onPostExecute(result);
    }

    protected void onSuccess() {
        magiskManager.updateCheckDone.trigger();
        new LoadModules(activity).exec();

        new AlertDialogBuilder(activity)
                .setTitle(R.string.reboot_title)
                .setMessage(R.string.reboot_msg)
                .setPositiveButton(R.string.reboot, (dialogInterface, i) -> Shell.su(true, "reboot"))
                .setNegativeButton(R.string.no_thanks, null)
                .show();
    }
}
