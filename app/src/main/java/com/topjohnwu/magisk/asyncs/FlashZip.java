package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.net.Uri;
import android.text.TextUtils;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.container.AdaptiveList;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.ZipUtils;

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
    private AdaptiveList<String> mList;

    public FlashZip(Activity context, Uri uri, AdaptiveList<String> list) {
        super(context);
        mUri = uri;
        mList = list;
        mCachedFile = new File(context.getCacheDir(), "install.zip");
    }

    private boolean unzipAndCheck() throws Exception {
        ZipUtils.unzip(mCachedFile, mCachedFile.getParentFile(), "META-INF/com/google/android", true);
        List<String> ret = Utils.readFile(new File(mCachedFile.getParentFile(), "updater-script").getPath());
        return Utils.isValidShellResponse(ret) && ret.get(0).contains("#MAGISK");
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
    protected Integer doInBackground(Void... voids) {
        MagiskManager mm = MagiskManager.get();
        try {
            mList.add("- Copying zip to temp directory");

            mCachedFile.delete();
            try (
                InputStream in = mm.getContentResolver().openInputStream(mUri);
                OutputStream out = new BufferedOutputStream(new FileOutputStream(mCachedFile))
            ) {
                if (in == null) throw new FileNotFoundException();
                InputStream buf= new BufferedInputStream(in);
                Utils.inToOut(buf, out);
            } catch (FileNotFoundException e) {
                mList.add("! Invalid Uri");
                throw e;
            } catch (IOException e) {
                mList.add("! Cannot copy to cache");
                throw e;
            }
            if (!unzipAndCheck()) return 0;
            mList.add("- Installing " + Utils.getNameFromUri(mm, mUri));
            Shell.su(mList,
                    "cd " + mCachedFile.getParent(),
                    "BOOTMODE=true sh update-binary dummy 1 " + mCachedFile +
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
        MagiskManager mm = MagiskManager.get();
        Shell.su_raw(
                "rm -rf " + mCachedFile.getParent(),
                "rm -rf " + MagiskManager.TMP_FOLDER_PATH
        );
        switch (result) {
            case -1:
                mList.add(mm.getString(R.string.install_error));
                Utils.showUriSnack(getActivity(), mUri);
                break;
            case 0:
                mList.add(mm.getString(R.string.invalid_zip));
                break;
            case 1:
                // Success
                new LoadModules().exec();
                break;
        }
        super.onPostExecute(result);
    }
}
