package com.topjohnwu.magisk.module;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;

public class Module extends BaseModule {

    private String mRemoveFile;
    private String mDisableFile;

    private String mZipUrl, mLogUrl;
    private boolean mEnable, mRemove, mUpdateAvailable = false, mIsInstalled;

    public Module(Context context, String path) {

        super();
        parseProps(Utils.readFile(path + "/module.prop"));

        mRemoveFile = path + "/remove";
        mDisableFile = path + "/disable";

        if (mId == null) {
            int sep = path.lastIndexOf('/');
            mId = path.substring(sep + 1);
        }

        if (mName == null)
            mName = mId;

        if (mDescription == null)
            mDescription = context.getString(R.string.no_info_provided);

        if (mVersion == null)
            mVersion = context.getString(R.string.no_info_provided);

        Log.d("Magisk","Module: Loaded module with ID of " + mId );

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);

        if (this.mId != null && !this.mId.isEmpty()) {
            String preferenceString = "repo_" + this.mId;
            String preferenceKey = prefs.getString(preferenceString,"nope");
            Log.d("Magisk", "Module: Checking for preference named " + preferenceString);
            if (!preferenceKey.equals("nope")) {
                Log.d("Magisk", "Module: repo_" + mId + " found.");
                String entryString = prefs.getString("repo_" + mId, "");
                String[] subStrings = entryString.split("\n");
                for (String subKeys : subStrings) {
                    String[] idEntry = subKeys.split("=", 2);
                    if (idEntry[0].equals("id")) {
                        if (idEntry.length != 2) {
                            continue;
                        }

                        if (idEntry[1].equals(mId)) {
                            Log.d("Magisk", "Module: Hey, I know " + mId + " is online...");
                            mIsInstalled = true;
                        } else mIsInstalled = false;
                    }
                    if (idEntry[0].equals("logUrl")) {
                        mLogUrl = idEntry[1];
                    }
                    if (idEntry[0].equals("support")) {
                        mSupportUrl = idEntry[1];
                    }
                    if (idEntry[0].equals("zipUrl")) {
                        mZipUrl = idEntry[1];
                    }
                    if (idEntry[0].equals("donate")) {
                        mDonateUrl = idEntry[1];
                    }

                    if (idEntry[0].equals("versionCode")) {
                        if (idEntry.length != 2) {
                            continue;
                        }

                        if (Integer.valueOf(idEntry[1]) > mVersionCode) {
                            mUpdateAvailable = true;
                            Log.d("Magisk", "Module: Hey, I have an update...");
                        } else mUpdateAvailable = false;
                    }
                }


            }

            SharedPreferences.Editor editor = prefs.edit();
            if (mIsInstalled) {
                editor.putBoolean("repo-isInstalled_" + mId, true);
            } else {
                editor.putBoolean("repo-isInstalled_" + mId, false);
            }

            if (mUpdateAvailable) {
                editor.putBoolean("repo-canUpdate_" + mId, true);
            } else {
                editor.putBoolean("repo-canUpdate_" + mId, false);
            }
            editor.apply();
        }

        mEnable = !Utils.fileExist(mDisableFile);
        mRemove = Utils.fileExist(mRemoveFile);

    }

    public String getmLogUrl() {return mLogUrl; }

    public void createDisableFile() {
        mEnable = !Utils.createFile(mDisableFile);
    }

    public void removeDisableFile() {
        mEnable = Utils.removeFile(mDisableFile);
    }

    public boolean isEnabled() {
        return mEnable;
    }

    public void createRemoveFile() {
        mRemove = Utils.createFile(mRemoveFile);
    }

    public void deleteRemoveFile() {
        mRemove = !Utils.removeFile(mRemoveFile);
    }

    public boolean willBeRemoved() {
        return mRemove;
    }

    public String getmZipUrl() { return mZipUrl; }

    public boolean isUpdateAvailable() { return mUpdateAvailable; }

}