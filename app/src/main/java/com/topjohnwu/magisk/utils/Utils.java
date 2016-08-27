package com.topjohnwu.magisk.utils;

import android.app.ProgressDialog;
import android.content.Context;
import android.os.AsyncTask;
import android.support.v7.app.AlertDialog;
import android.widget.Toast;

import com.topjohnwu.magisk.R;

import java.util.List;

public class Utils {

    public static boolean fileExist(String path) {
        List<String> ret;
        ret = Shell.sh("if [ -f " + path + " ]; then echo true; else echo false; fi");
        if (!Boolean.parseBoolean(ret.get(0)) && Shell.rootAccess()) ret = Shell.su("if [ -f " + path + " ]; then echo true; else echo false; fi");
        return Boolean.parseBoolean(ret.get(0));
    }

    public static boolean createFile(String path) {
        if (!Shell.rootAccess()) {
            return false;
        } else {
            return Boolean.parseBoolean(Shell.su("touch " + path + " 2>/dev/null; if [ -f " + path + " ]; then echo true; else echo false; fi").get(0));
        }
    }

    public static boolean removeFile(String path) {
        if (!Shell.rootAccess()) {
            return false;
        } else {
            return Boolean.parseBoolean(Shell.su("rm -f " + path + " 2>/dev/null; if [ -f " + path + " ]; then echo false; else echo true; fi").get(0));
        }
    }

    public static List<String> getModList(String path) {
        List<String> ret;
        ret = Shell.sh("find " + path + " -type d -maxdepth 1 | while read ITEM ; do if [ -f $ITEM/module.prop ]; then echo $ITEM; fi; done");
        if (ret.isEmpty() && Shell.rootAccess()) ret = Shell.su("find " + path + " -type d -maxdepth 1 | while read ITEM ; do if [ -f $ITEM/module.prop ]; then echo $ITEM; fi; done");
        return ret;
    }

    public static List<String> readFile(String path) {
        List<String> ret;
        ret = Shell.sh("cat " + path);
        if (ret.isEmpty() && Shell.rootAccess()) ret = Shell.su("cat " + path);
        return ret;
    }

    public static class flashZIP extends AsyncTask<Void, Void, Boolean> {

        private String mPath;
        private ProgressDialog progress;
        Context mContext;

        public flashZIP(String path, Context context) {
            mPath = path;
            mContext = context;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            progress = ProgressDialog.show(mContext, "Installing", "Patching boot image for Magisk....");
        }

        @Override
        protected Boolean doInBackground(Void... voids) {
            if (!Shell.rootAccess()) {
                return false;
            } else {
                Shell.su(
                        "rm -rf /data/tmp",
                        "mkdir -p /data/tmp",
                        "cp -af " + mPath + " /data/tmp/install.zip",
                        "unzip -o /data/tmp/install.zip META-INF/com/google/android/* -d /data/tmp",
                        "BOOTMODE=true sh /data/tmp/META-INF/com/google/android/update-binary dummy 1 /data/tmp/install.zip"
                );
                return Boolean.parseBoolean(Shell.su("if [ $? -eq 0 ]; then echo true; else echo false; fi").get(0));
            }
        }

        @Override
        protected void onPostExecute(Boolean result) {
            super.onPostExecute(result);
            Shell.su("rm -rf /data/tmp");
            progress.dismiss();
            if (!result) {
                Toast.makeText(mContext, "Installation failed...", Toast.LENGTH_LONG).show();
                return;
            }
            new AlertDialog.Builder(mContext)
                    .setTitle("Reboot now?")
                    .setPositiveButton("Yes", (dialogInterface1, i1) -> Toast.makeText(mContext, "Reboot...", Toast.LENGTH_LONG).show())
                    .setNegativeButton(R.string.no_thanks, null)
                    .show();
        }
    }

}
