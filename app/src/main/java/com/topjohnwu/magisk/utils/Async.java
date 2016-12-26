package com.topjohnwu.magisk.utils;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.preference.PreferenceManager;
import android.provider.OpenableColumns;
import android.support.v7.app.AlertDialog;
import android.util.Log;
import android.widget.Toast;

import com.topjohnwu.magisk.InstallFragment;
import com.topjohnwu.magisk.ModulesFragment;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.ReposFragment;
import com.topjohnwu.magisk.StatusFragment;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;

public class Async {

    public abstract static class RootTask<Params, Progress, Result> extends AsyncTask<Params, Progress, Result> {
        @SafeVarargs
        public final void exec(Params... params) {
            executeOnExecutor(AsyncTask.SERIAL_EXECUTOR, params);
        }
    }

    public abstract static class NormalTask<Params, Progress, Result> extends AsyncTask<Params, Progress, Result> {
        @SafeVarargs
        public final void exec(Params... params) {
            executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, params);
        }
    }

    public static final String UPDATE_JSON = "https://raw.githubusercontent.com/topjohnwu/MagiskManager/updates/magisk_update.json";
    public static final String MAGISK_HIDE_PATH = "/magisk/.core/magiskhide/";
    public static final String TMP_FOLDER_PATH = "/dev/tmp";

    public static class CheckUpdates extends NormalTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            String jsonStr = WebRequest.makeWebServiceCall(UPDATE_JSON, WebRequest.GET);
            try {
                JSONObject json = new JSONObject(jsonStr);

                JSONObject magisk = json.getJSONObject("magisk");

                StatusFragment.remoteMagiskVersion = magisk.getDouble("versionCode");
                StatusFragment.magiskLink = magisk.getString("link");
                StatusFragment.magiskChangelog = magisk.getString("changelog");

            } catch (JSONException ignored) {}
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            StatusFragment.updateCheckDone.trigger();
        }
    }

    public static void checkSafetyNet(Context context) {
        new SafetyNetHelper(context) {
            @Override
            public void handleResults(int i) {
                StatusFragment.SNCheckResult = i;
                StatusFragment.safetyNetDone.trigger();
            }
        }.requestTest();
    }

    public static class LoadModules extends RootTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            ModuleHelper.createModuleMap();
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            ModulesFragment.moduleLoadDone.trigger();
        }
    }

    public static class LoadRepos extends NormalTask<Void, Void, Void> {

        private Context mContext;

        public LoadRepos(Context context) {
            mContext = context;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            ModuleHelper.createRepoMap(mContext);
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            ReposFragment.repoLoadDone.trigger();
        }
    }

    public static class FlashZIP extends RootTask<Void, String, Integer> {

        protected Uri mUri;
        protected File mCachedFile;
        private String mFilename;
        protected ProgressDialog progress;
        private Context mContext;

        public FlashZIP(Context context, Uri uri, String filename) {
            mContext = context;
            mUri = uri;
            mFilename = filename;
        }

        public FlashZIP(Context context, Uri uri) {
            mContext = context;
            mUri = uri;

            // Try to get the filename ourselves
            Cursor c = mContext.getContentResolver().query(uri, null, null, null, null);
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

        protected void preProcessing() throws Throwable {}

        private void copyToCache() throws Throwable {
            try {
                InputStream in = mContext.getContentResolver().openInputStream(mUri);
                mCachedFile = new File(mContext.getCacheDir().getAbsolutePath() + "/install.zip");
                if (mCachedFile.exists() && !mCachedFile.delete()) {
                    throw new IOException();
                }
                OutputStream outputStream = new FileOutputStream(mCachedFile);
                byte buffer[] = new byte[1024];
                int length;
                while ((length = in.read(buffer)) > 0) {
                    outputStream.write(buffer, 0, length);
                }
                outputStream.close();
                Logger.dev("FlashZip: File created successfully - " + mCachedFile.getPath());
                in.close();
            } catch (FileNotFoundException e) {
                Log.e(Logger.TAG, "FlashZip: Invalid Uri");
                throw e;
            } catch (IOException e) {
                Log.e(Logger.TAG, "FlashZip: Error in creating file");
                throw e;
            }
        }

        protected boolean unzipAndCheck() {
            ZipUtils.unzip(mCachedFile, mCachedFile.getParentFile(), "META-INF/com/google/android");
            return Utils.readFile(mCachedFile.getParent() + "/META-INF/com/google/android/updater-script")
                    .get(0).contains("#MAGISK");
        }

        @Override
        protected void onPreExecute() {
            progress = new ProgressDialog(mContext);
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
            if (Shell.rootAccess()) {
                publishProgress(mContext.getString(R.string.zip_install_progress_msg, mFilename));
                ret = Shell.su(
                        "BOOTMODE=true sh " + mCachedFile.getParent() +
                                "/META-INF/com/google/android/update-binary dummy 1 " + mCachedFile.getPath(),
                        "if [ $? -eq 0 ]; then echo true; else echo false; fi"
                );
                Logger.dev("FlashZip: Console log:");
                for (String line : ret) {
                    Logger.dev(line);
                }
                Shell.su(
                        "rm -rf " + mCachedFile.getParent() + "/*",
                        "rm -rf " + TMP_FOLDER_PATH
                );
            } else {
                if (mCachedFile != null && mCachedFile.exists() && !mCachedFile.delete()) {
                    Utils.removeItem(mCachedFile.getPath());
                }
                return -1;
            }
            if (ret != null && Boolean.parseBoolean(ret.get(ret.size() - 1))) {
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
                    Toast.makeText(mContext, mContext.getString(R.string.install_error), Toast.LENGTH_LONG).show();
                    Toast.makeText(mContext, mContext.getString(R.string.manual_install_1, mUri.getPath()), Toast.LENGTH_LONG).show();
                    Toast.makeText(mContext, mContext.getString(R.string.manual_install_2), Toast.LENGTH_LONG).show();
                    break;
                case 0:
                    Toast.makeText(mContext, mContext.getString(R.string.invalid_zip), Toast.LENGTH_LONG).show();
                    break;
                case 1:
                    onSuccess();
                    break;
            }
        }

        protected void onSuccess() {
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);

            StatusFragment.updateCheckDone.trigger();
            new LoadModules().exec();

            AlertDialog.Builder builder;
            String theme = prefs.getString("theme", "");
            if (theme.equals("Dark")) {
                builder = new AlertDialog.Builder(mContext, R.style.AlertDialog_dh);
            } else {
                builder = new AlertDialog.Builder(mContext);
            }
            builder
                    .setTitle(R.string.reboot_title)
                    .setMessage(R.string.reboot_msg)
                    .setPositiveButton(R.string.reboot, (dialogInterface1, i) -> Shell.sh("su -c reboot"))
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        }
    }

    public static class MagiskHide extends RootTask<Object, Void, Void> {
        @Override
        protected Void doInBackground(Object... params) {
            boolean add = (boolean) params[0];
            String packageName = (String) params[1];
            Shell.su(MAGISK_HIDE_PATH + (add ? "add " : "rm ") + packageName);
            return null;
        }

        public void add(CharSequence packageName) {
            exec(true, packageName);
        }

        public void rm(CharSequence packageName) {
            exec(false, packageName);
        }

    }

    public static class GetBootBlocks extends RootTask<Void, Void, Void> {
        @Override
        protected Void doInBackground(Void... params) {
            InstallFragment.blockList = Shell.su("ls /dev/block | grep mmc");
            if (InstallFragment.bootBlock == null) {
                InstallFragment.bootBlock = Utils.detectBootImage();
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            InstallFragment.blockDetectionDone.trigger();
        }
    }
}
