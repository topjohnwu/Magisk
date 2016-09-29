package com.topjohnwu.magisk.utils;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.provider.OpenableColumns;
import android.support.v7.app.AlertDialog;
import android.util.Log;
import android.widget.Toast;

import com.topjohnwu.magisk.MagiskFragment;
import com.topjohnwu.magisk.R;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;

public class Async {

    public static final String UPDATE_JSON = "https://raw.githubusercontent.com/topjohnwu/MagiskManager/updates/magisk_update.json";

    public static class constructEnv extends AsyncTask<Void, Void, Void> {

        Context mContext;

        public constructEnv(Context context) {
            mContext = context;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            String toolPath = mContext.getApplicationInfo().dataDir + "/tools";
            String busybox = mContext.getApplicationInfo().dataDir + "/lib/libbusybox.so";
            String zip = mContext.getApplicationInfo().dataDir + "/lib/libzip.so";
            if (Shell.rootAccess()) {
                if (!Utils.itemExist(toolPath)) {
                    Shell.sh(
                            "rm -rf " + toolPath,
                            "mkdir " + toolPath,
                            "chmod 755 " + toolPath,
                            "cd " + toolPath,
                            "ln -s " + busybox + " busybox",
                            "for tool in $(./busybox --list); do",
                            "ln -s " + busybox + " $tool",
                            "done",
                            "rm -f su sh",
                            "ln -s " + zip + " zip"
                    );
                }
            }

            return null;
        }
    }

    public static class CheckUpdates extends AsyncTask<Void, Void, Void> {

        private Context mContext;

        public CheckUpdates(Context context) {
            mContext = context;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            String jsonStr = WebRequest.makeWebServiceCall(UPDATE_JSON, WebRequest.GET);
            try {
                JSONObject json = new JSONObject(jsonStr);

                JSONObject magisk = json.getJSONObject("magisk");
                JSONObject app = json.getJSONObject("app");

                MagiskFragment.remoteMagiskVersion = magisk.getInt("versionCode");
                MagiskFragment.magiskLink = magisk.getString("link");
                MagiskFragment.magiskChangelog = magisk.getString("changelog");

                MagiskFragment.remoteAppVersion = app.getString("version");
                MagiskFragment.remoteAppVersionCode = app.getInt("versionCode");
                MagiskFragment.appLink = app.getString("link");
                MagiskFragment.appChangelog = app.getString("changelog");

            } catch (JSONException ignored) {
                Logger.dev("JSON error!");
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
            prefs.edit().putBoolean("update_check_done", true).apply();
        }
    }

    public static class LoadModules extends AsyncTask<Void, Void, Void> {

        private Context mContext;

        public LoadModules(Context context) {
            mContext = context;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            ModuleHelper.createModuleMap();
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
            prefs.edit().putBoolean("module_done", true).apply();
        }
    }

    public static class LoadRepos extends AsyncTask<Void, Void, Void> {

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
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
            prefs.edit().putBoolean("repo_done", true).apply();
        }
    }

    public static class FlashZIP extends AsyncTask<Void, Void, Integer> {

        private String mName;
        protected Uri mUri;
        private ProgressDialog progress;
        protected File mFile, sdFile;
        private Context mContext;
        private boolean copyToSD;

        public FlashZIP(Context context, Uri uri, String name) {
            mContext = context;
            mUri = uri;
            mName = name;
            copyToSD = true;
        }

        public FlashZIP(Context context, Uri uri) {
            mContext = context;
            mUri = uri;
            Cursor c = mContext.getContentResolver().query(uri, null, null, null, null);
            if (c != null) {
                int nameIndex = c.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                c.moveToFirst();
                if (nameIndex != -1) {
                    mName = c.getString(nameIndex);
                }
                c.close();
            }
            if (mName == null) {
                int idx = uri.getPath().lastIndexOf('/');
                mName = uri.getPath().substring(idx + 1);
            }
            copyToSD = false;
        }

        private void createFileFromInputStream(InputStream inputStream, File file) throws IOException {
            if (file.exists() && !file.delete()) {
                throw new IOException();
            }
            file.setWritable(true, false);
            OutputStream outputStream = new FileOutputStream(file);
            byte buffer[] = new byte[1024];
            int length;

            while ((length = inputStream.read(buffer)) > 0) {
                outputStream.write(buffer, 0, length);
            }

            outputStream.close();
            Logger.dev("FlashZip: File created successfully - " + file.getPath());
        }

        protected void preProcessing() throws Throwable {
            try {
                InputStream in = mContext.getContentResolver().openInputStream(mUri);
                mFile = new File(mContext.getCacheDir().getAbsolutePath() + "/install.zip");
                createFileFromInputStream(in, mFile);
                in.close();
            } catch (FileNotFoundException e) {
                Log.e("Magisk", "FlashZip: Invalid Uri");
                throw e;
            } catch (IOException e) {
                Log.e("Magisk", "FlashZip: Error in creating file");
                throw e;
            }
        }

        @Override
        protected void onPreExecute() {
            progress = ProgressDialog.show(mContext, mContext.getString(R.string.zip_install_progress_title), mContext.getString(R.string.zip_install_progress_msg, mName));
        }

        @Override
        protected Integer doInBackground(Void... voids) {
            Logger.dev("FlashZip Running... " + mName);
            List<String> ret = null;
            if (Shell.rootAccess()) {
                try {
                    preProcessing();
                } catch (Throwable e) {
                    this.cancel(true);
                    progress.cancel();
                    e.printStackTrace();
                    return -1;
                }
                ret = Shell.su(
                        "unzip -o " + mFile.getPath() + " META-INF/com/google/android/* -d " + mFile.getParent(),
                        "if [ \"$(cat " + mFile.getParent() + "/META-INF/com/google/android/updater-script)\" = \"#MAGISK\" ]; then echo true; else echo false; fi"
                );
                if (! Boolean.parseBoolean(ret.get(ret.size() - 1))) {
                    return 0;
                }
                ret = Shell.su(
                        "BOOTMODE=true sh " + mFile.getParent() + "/META-INF/com/google/android/update-binary dummy 1 "+ mFile.getPath(),
                        "if [ $? -eq 0 ]; then echo true; else echo false; fi",
                        "rm -rf " + mFile.getParent() + "/META-INF"
                );
                Logger.dev("FlashZip: Console log:");
                for (String line : ret) {
                    Logger.dev(line);
                }
            }
            // Copy the file to sdcard
            if (copyToSD && mFile != null) {
                sdFile = new File(Environment.getExternalStorageDirectory() + "/MagiskManager/" + (mName.contains(".zip") ? mName : mName + ".zip").replace(" ", "_"));
                if ((!sdFile.getParentFile().exists() && !sdFile.getParentFile().mkdirs()) || (sdFile.exists() && !sdFile.delete())) {
                    sdFile = null;
                } else {
                    try {
                        FileInputStream in = new FileInputStream(mFile);
                        createFileFromInputStream(in, sdFile);
                        in.close();
                        mFile.delete();
                    } catch (IOException e) {
                        // Use the badass way :)
                        Shell.su("cp -af " + mFile.getPath() + " " + sdFile.getPath());
                        if (!sdFile.exists()) {
                            sdFile = null;
                        }
                    }
                }
                if (mFile.exists() && !mFile.delete()) {
                    Shell.su("rm -f " + mFile.getPath());
                }
            }
            if (ret != null && Boolean.parseBoolean(ret.get(ret.size() - 1))) {
                return 1;
            }
            return -1;
        }

        // -1 = error; 0 = invalid zip; 1 = success
        @Override
        protected void onPostExecute(Integer result) {
            super.onPostExecute(result);
            progress.dismiss();
            switch (result) {
                case -1:
                    if (sdFile == null) {
                        Toast.makeText(mContext, mContext.getString(R.string.install_error), Toast.LENGTH_LONG).show();
                    } else {
                        Toast.makeText(mContext, mContext.getString(R.string.manual_install, mFile.getAbsolutePath()), Toast.LENGTH_LONG).show();
                    }
                    break;
                case 0:
                    Toast.makeText(mContext, mContext.getString(R.string.invalid_zip), Toast.LENGTH_LONG).show();
                    break;
                case 1:
                    done();
                    break;
            }
        }

        protected void done() {
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
            prefs.edit().putBoolean("module_done", false).apply();
            new LoadModules(mContext).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);

            AlertDialog.Builder builder;
            String theme = PreferenceManager.getDefaultSharedPreferences(mContext).getString("theme", "");
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

    public static class LinkBusyBox extends AsyncTask<Void, Void, Void> {

        private boolean link;

        public LinkBusyBox(boolean b) {
            link = b;
        }

        @Override
        protected Void doInBackground(Void... voids) {
            if (link) {
                Shell.su(
                        "rm -rf /magisk/.core/busybox",
                        "ln -s /data/busybox /magisk/.core/busybox"
                );
            } else {
                Shell.su("rm -rf /magisk/.core/busybox");
            }
            return null;
        }
    }
}
