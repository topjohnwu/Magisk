package com.topjohnwu.magisk.module;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

import com.topjohnwu.magisk.utils.Utils;

import java.util.Map;

public class Module {

    private String mRemoveFile;
    private String mDisableFile;

    private String mName = null;
    private String mVersion = "(No version provided)";
    private String mDescription = "(No description provided)";
    private String mUrl,mSupportUrl,mDonateUrl,mZipUrl,mBaseUrl,mManifestUrl,mAuthor;
    private boolean mEnable, mRemove,mUpdateAvailable,mIsOnline;


    private String mId;
    private int mVersionCode;

    public Module(String path, Context context) {

        mRemoveFile = path + "/remove";
        mDisableFile = path + "/disable";
        for (String line : Utils.readFile(path + "/module.prop")) {
            String[] props = line.split("=", 2);
            if (props.length != 2) {
                continue;
            }

            String key = props[0].trim();
            if (key.charAt(0) == '#') {
                continue;
            }

            String value = props[1].trim();
            switch (props[0]) {
                case "versionCode":
                    this.mVersionCode = Integer.valueOf(props[1]);
                    break;
                case "name":
                    this.mName = value;
                    break;
                case "author":
                    this.mAuthor = value;
                    break;
                case "id":
                    this.mId = value;
                    break;
                case "version":
                    this.mVersion = value;
                    break;
                case "description":
                    this.mDescription = value;
                    break;
                case "donate":
                    this.mDonateUrl = value;
                    break;
                case "support":
                    this.mSupportUrl = value;
                    break;
                case "donateUrl":
                    this.mDonateUrl = value;
                    break;
                case "zipUrl":
                    this.mZipUrl = value;
                    break;
                case "baseUrl":
                    this.mBaseUrl = value;
                    break;
                case "manifestUrl":
                    this.mManifestUrl = value;
                    break;
                default:
                    Log.d("Magisk", "Module: Manifest string not recognized: " + props[0]);
                    break;
            }


        }

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
                            Log.d("Magisk", "Module: Hey, I know I'm online...");
                            mIsOnline = true;
                        } else mIsOnline = false;
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
            if (mIsOnline) {
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

        if (mName == null) {
            int sep = path.lastIndexOf('/');
            mName = path.substring(sep + 1);
            mId = mName;
        }

        mEnable = !Utils.fileExist(mDisableFile);
        mRemove = Utils.fileExist(mRemoveFile);

    }

    public Module(Repo repo) {

        mName = repo.getName();
        mVersion = repo.getmVersion();
        mDescription = repo.getDescription();
        mId = repo.getId();
        mVersionCode = repo.getmVersionCode();
        mUrl = repo.getmZipUrl();
        mEnable = true;
        mRemove = false;

    }



    public String getName() {
        return mName;
    }

    public String getVersion() {
        return mVersion;
    }

    public String getDescription() {
        return mDescription;
    }

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

    public boolean isOnline() {return mIsOnline; }

    public boolean isUpdateAvailable() { return mUpdateAvailable; };

}