package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.app.ProgressDialog;
import android.database.Cursor;
import android.net.Uri;
import android.provider.OpenableColumns;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
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

public class FlashZIP extends SerialTask<Void, String, Integer> {

    protected Uri mUri;
    protected File mCachedFile;

    private String mFilename;
    private ProgressDialog progress;

    public FlashZIP(Activity context, Uri uri, String filename) {
        super(context);
        mUri = uri;
        mFilename = filename;
    }

    public FlashZIP(Activity context, Uri uri) {
        super(context);
        mUri = uri;

        // Try to get the filename ourselves
        Cursor c = magiskManager.getContentResolver().query(uri, null, null, null, null);
        if (c != null) {
            int nameIndex = c.getColumnIndex(OpenableColumns.DISPLAY_NAME);
            c.moveToFirst();
            if (nameIndex != -1) {
                mFilename = c.getString(nameIndex);
            }
            c.close();
        }
        if (mFilename == null) {
            int idx = uri.getPath().lastIndexOf('/');
            mFilename = uri.getPath().substring(idx + 1);
        }
    }

    protected void preProcessing() throws Throwable {
    }

    protected void copyToCache() throws Throwable {
        publishProgress(magiskManager.getString(R.string.copying_msg));
        mCachedFile = new File(magiskManager.getCacheDir().getAbsolutePath() + "/install.zip");
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
            while ((length = in.read(buffer)) > 0) {
                outputStream.write(buffer, 0, length);
            }
            Logger.dev("FlashZip: File created successfully - " + mCachedFile.getPath());
        } catch (FileNotFoundException e) {
            Logger.error("FlashZip: Invalid Uri");
            throw e;
        } catch (IOException e) {
            Logger.error("FlashZip: Error in creating file");
            throw e;
        }
    }

    protected boolean unzipAndCheck() {
        ZipUtils.unzip(mCachedFile, mCachedFile.getParentFile(), "META-INF/com/google/android");
        List<String> ret;
        ret = Utils.readFile(mCachedFile.getParent() + "/META-INF/com/google/android/updater-script");
        return Utils.isValidShellResponse(ret) && ret.get(0).contains("#MAGISK");
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
    protected Integer doInBackground(Void... voids) {
        Logger.dev("FlashZip Running... " + mFilename);
        List<String> ret;
        try {
            preProcessing();
            copyToCache();
        } catch (Throwable e) {
            e.printStackTrace();
            return -1;
        }
        if (!unzipAndCheck()) return 0;
        publishProgress(magiskManager.getString(R.string.zip_install_progress_msg, mFilename));
        ret = Shell.su(
                "BOOTMODE=true sh " + mCachedFile.getParent() +
                        "/META-INF/com/google/android/update-binary dummy 1 " + mCachedFile.getPath(),
                "if [ $? -eq 0 ]; then echo true; else echo false; fi"
        );
        if (!Utils.isValidShellResponse(ret)) return -1;
        Logger.dev("FlashZip: Console log:");
        for (String line : ret) {
            Logger.dev(line);
        }
        Shell.su(
                "rm -rf " + mCachedFile.getParent() + "/*",
                "rm -rf " + MagiskManager.TMP_FOLDER_PATH
        );
        if (Boolean.parseBoolean(ret.get(ret.size() - 1))) {
            return 1;
        }
        return -1;
    }

    // -1 = error, manual install; 0 = invalid zip; 1 = success
    @Override
    protected void onPostExecute(Integer result) {
        super.onPostExecute(result);
        progress.dismiss();
        switch (result) {
            case -1:
                Toast.makeText(magiskManager, magiskManager.getString(R.string.install_error), Toast.LENGTH_LONG).show();
                Toast.makeText(magiskManager, magiskManager.getString(R.string.manual_install_1, mUri.getPath()), Toast.LENGTH_LONG).show();
                Toast.makeText(magiskManager, magiskManager.getString(R.string.manual_install_2), Toast.LENGTH_LONG).show();
                break;
            case 0:
                Toast.makeText(magiskManager, magiskManager.getString(R.string.invalid_zip), Toast.LENGTH_LONG).show();
                break;
            case 1:
                onSuccess();
                break;
        }
    }

    protected void onSuccess() {
        magiskManager.updateCheckDone.trigger();
        new LoadModules(activity).exec();

        Utils.getAlertDialogBuilder(activity)
                .setTitle(R.string.reboot_title)
                .setMessage(R.string.reboot_msg)
                .setPositiveButton(R.string.reboot, (dialogInterface, i) -> Shell.su(true, "reboot"))
                .setNegativeButton(R.string.no_thanks, null)
                .show();
    }
}
