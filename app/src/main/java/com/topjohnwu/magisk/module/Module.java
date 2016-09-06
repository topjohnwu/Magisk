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
                    this.mName = props[1];
                    break;
                case "author":
                    this.mAuthor = props[1];
                    break;
                case "id":
                    this.mId = props[1];
                    break;
                case "version":
                    this.mVersion = props[1];
                    break;
                case "description":
                    this.mDescription = props[1];
                    break;
                case "donate":
                    this.mDonateUrl = props[1];
                    break;
                case "support":
                    this.mSupportUrl = props[1];
                    break;
                case "donateUrl":
                    this.mDonateUrl = props[1];
                    break;
                case "zipUrl":
                    this.mZipUrl = props[1];
                    break;
                case "baseUrl":
                    this.mBaseUrl = props[1];
                    break;
                case "manifestUrl":
                    this.mManifestUrl = props[1];
                    break;
                default:
                    Log.d("Magisk", "Manifest string not recognized: " + props[0]);
                    break;
            }

            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
            if (mId != null) {
                Log.d("Magisk", "Module: Checking for preference named repo_" + mId);
                if (prefs.contains("repo_" + mId)) {
                    String entryString = prefs.getString("repo_" + mId, "");


                    String entryName = "repo" + mId;

                    String[] subStrings = entryString.split("\n");
                    for (String subKeys : subStrings) {
                        String[] idEntry = subKeys.split("=", 2);
                        Log.d("Magisk", "Module: Checking entry strings. Key is " + idEntry[0] + " and value is " + idEntry[1]);
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
                    editor.putBoolean("repo_isInstalled_" + mId, true);

                } else {
                    editor.putBoolean("repo_isInstalled_" + mId, false);
                }
                editor.apply();
            }
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
        mId = "foo";
        mVersionCode = 111;
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