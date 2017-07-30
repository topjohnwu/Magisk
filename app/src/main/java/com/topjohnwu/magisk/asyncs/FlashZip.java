package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.content.Context;
import android.net.Uri;
import android.text.TextUtils;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.AdaptiveList;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ZipUtils;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;

public class FlashZip extends ParallelTask<Void, String, Integer> {

    private Uri mUri;
    private File mCachedFile, mScriptFile, mCheckFile;

    private String mFilename;
    private AdaptiveList<String> mList;

    public FlashZip(Activity context, Uri uri, AdaptiveList<String> list) {
        super(context);
        mUri = uri;
        mList = list;

        mCachedFile = new File(context.getCacheDir(), "install.zip");
        mScriptFile = new File(context.getCacheDir(), "/META-INF/com/google/android/update-binary");
        mCheckFile = new File(mScriptFile.getParent(), "updater-script");

        // Try to get the filename ourselves
        mFilename = Utils.getNameFromUri(context, mUri);
    }

    private boolean unzipAndCheck() throws Exception {
        ZipUtils.unzip(mCachedFile, mCachedFile.getParentFile(), "META-INF/com/google/android");
        List<String> ret = Utils.readFile(getShell(), mCheckFile.getPath());
        return Utils.isValidShellResponse(ret) && ret.get(0).contains("#MAGISK");
    }

    @Override
    protected void onPreExecute() {
        // UI updates must run in the UI thread
        mList.setCallback(this::publishProgress);
    }

    @Override
    protected void onProgressUpdate(String... values) {
        mList.updateView();
    }

    @Override
    protected Integer doInBackground(Void... voids) {
        MagiskManager magiskManager = getMagiskManager();
        if (magiskManager == null) return -1;
        try {
            mList.add(magiskManager.getString(R.string.copying_msg));

            mCachedFile.delete();
            try (
                InputStream in = magiskManager.getContentResolver().openInputStream(mUri);
                OutputStream outputStream = new FileOutputStream(mCachedFile)
            ) {
                if (in == null) throw new FileNotFoundException();
                byte buffer[] = new byte[1024];
                int length;
                while ((length = in.read(buffer)) > 0)
                    outputStream.write(buffer, 0, length);
            } catch (FileNotFoundException e) {
                mList.add("! Invalid Uri");
                throw e;
            } catch (IOException e) {
                mList.add("! Cannot copy to cache");
                throw e;
            }
            if (!unzipAndCheck()) return 0;
            mList.add(magiskManager.getString(R.string.zip_install_progress_msg, mFilename));
            magiskManager.shell.su(mList,
                    "BOOTMODE=true sh " + mScriptFile + " dummy 1 " + mCachedFile +
                            " && echo 'Success!' || echo 'Failed!'"
            );
            if (TextUtils.equals(mList.get(mList.size() - 1), "Success!"))
                return 1;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return -1;
    }

    // -1 = error, manual install; 0 = invalid zip; 1 = success
    @Override
    protected void onPostExecute(Integer result) {
        MagiskManager magiskManager = getMagiskManager();
        if (magiskManager == null) return;
        magiskManager.shell.su_raw(
                "rm -rf " + mCachedFile.getParent(),
                "rm -rf " + MagiskManager.TMP_FOLDER_PATH
        );
        switch (result) {
            case -1:
                mList.add(magiskManager.getString(R.string.install_error));
                Utils.showUriSnack(getActivity(), mUri);
                break;
            case 0:
                mList.add(magiskManager.getString(R.string.invalid_zip));
                break;
            case 1:
                // Success
                new LoadModules(magiskManager).exec();
                break;
        }
        super.onPostExecute(result);
    }
}
