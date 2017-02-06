package com.topjohnwu.magisk.utils;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.provider.OpenableColumns;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.adapters.ApplicationAdapter;
import com.topjohnwu.magisk.module.ModuleHelper;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

public class Async {

    public abstract static class RootTask<Params, Progress, Result> extends AsyncTask<Params, Progress, Result> {
        @SafeVarargs
        public final void exec(Params... params) {
            if (!Shell.rootAccess()) return;
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

        MagiskManager magiskManager;

        public CheckUpdates(MagiskManager context) {
            magiskManager = context;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            String jsonStr = WebService.request(UPDATE_JSON, WebService.GET);
            try {
                JSONObject json = new JSONObject(jsonStr);
                JSONObject magisk = json.getJSONObject("magisk");
                magiskManager.remoteMagiskVersion = magisk.getDouble("versionCode");
                magiskManager.magiskLink = magisk.getString("link");
                magiskManager.releaseNoteLink = magisk.getString("note");
            } catch (JSONException ignored) {}
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            magiskManager.updateCheckDone.trigger();
        }
    }

    public static void checkSafetyNet(MagiskManager magiskManager) {
        new SafetyNetHelper(magiskManager) {
            @Override
            public void handleResults(int i) {
                magiskManager.SNCheckResult = i;
                magiskManager.safetyNetDone.trigger();
            }
        }.requestTest();
    }

    public static class LoadModules extends RootTask<Void, Void, Void> {

        protected MagiskManager magiskManager;

        public LoadModules(MagiskManager context) {
            magiskManager = context;
        }
        @Override
        protected Void doInBackground(Void... voids) {
            ModuleHelper.createModuleMap(magiskManager);
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            magiskManager.moduleLoadDone.trigger();
        }
    }

    public static class LoadRepos extends NormalTask<Void, Void, Void> {

        private MagiskManager magiskManager;

        public LoadRepos(MagiskManager context) {
            magiskManager = context;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            ModuleHelper.createRepoMap(magiskManager);
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            magiskManager.repoLoadDone.trigger();
        }
    }

    public static class LoadApps extends RootTask<Void, Void, Void> {

        private MagiskManager magiskManager;

        public LoadApps(MagiskManager context) {
           magiskManager = context;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            PackageManager pm = magiskManager.getPackageManager();
            magiskManager.appList = pm.getInstalledApplications(0);
            for (Iterator<ApplicationInfo> i = magiskManager.appList.iterator(); i.hasNext(); ) {
                ApplicationInfo info = i.next();
                if (ApplicationAdapter.BLACKLIST.contains(info.packageName) || !info.enabled)
                    i.remove();
            }
            Collections.sort(magiskManager.appList, (a, b) -> a.loadLabel(pm).toString().toLowerCase()
                    .compareTo(b.loadLabel(pm).toString().toLowerCase()));
            magiskManager.magiskHideList = Shell.su(Async.MAGISK_HIDE_PATH + "list");
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            magiskManager.packageLoadDone.trigger();
        }
    }

    public static class FlashZIP extends RootTask<Void, String, Integer> {

        protected Uri mUri;
        protected File mCachedFile;
        private String mFilename;
        protected ProgressDialog progress;
        private Context mContext;
        private MagiskManager magiskManager;

        public FlashZIP(Context context, Uri uri, String filename) {
            mContext = context;
            magiskManager = (MagiskManager) context.getApplicationContext();
            mUri = uri;
            mFilename = filename;
        }

        public FlashZIP(Context context, Uri uri) {
            mContext = context;
            magiskManager = (MagiskManager) context.getApplicationContext();
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

        protected void preProcessing() throws Throwable {}

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
            progress = new ProgressDialog(magiskManager);
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
                    "rm -rf " + TMP_FOLDER_PATH
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
            new LoadModules(magiskManager).exec();

            Utils.getAlertDialogBuilder(mContext)
                    .setTitle(R.string.reboot_title)
                    .setMessage(R.string.reboot_msg)
                    .setPositiveButton(R.string.reboot, (dialogInterface, i) -> Shell.su(true, "reboot"))
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        }
    }

    public static class MagiskHide extends RootTask<Object, Void, Void> {

        private boolean newShell = false;

        public MagiskHide() {}

        public MagiskHide(boolean b) {
            newShell = b;
        }

        @Override
        protected Void doInBackground(Object... params) {
            String command = (String) params[0];
            Shell.su(newShell, MAGISK_HIDE_PATH + command);
            return null;
        }

        public void add(CharSequence packageName) {
            exec("add " + packageName);
        }

        public void rm(CharSequence packageName) {
            exec("rm " + packageName);
        }

        public void enable() {
            exec("enable");
        }

        public void disable() {
            exec("disable");
        }

    }

    public static class GetBootBlocks extends RootTask<Void, Void, Void> {

        MagiskManager magiskManager;

        public GetBootBlocks(MagiskManager context) {
            magiskManager = context;
        }

        @Override
        protected Void doInBackground(Void... params) {
            if (Shell.rootAccess()) {
                magiskManager.blockList = Shell.su("ls /dev/block | grep mmc");
                if (magiskManager.bootBlock == null)
                    magiskManager.bootBlock = Utils.detectBootImage();
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            magiskManager.blockDetectionDone.trigger();
        }
    }
}
