package com.topjohnwu.magisk.module;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;

public class Module extends BaseModule {

    private String mRemoveFile;
    private String mDisableFile;

    private String mZipUrl, mLogUrl;
    private boolean mEnable, mRemove, mUpdateAvailable = false;

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

        Logger.dh("Module id: " + mId);

        mEnable = !Utils.itemExist(mDisableFile);
        mRemove = Utils.itemExist(mRemoveFile);

    }

    public void checkUpdate() {
        Repo repo = RepoHelper.repoMap.get(mId);
        if (repo != null) {
            repo.setInstalled();
            if (repo.getVersionCode() > mVersionCode) {
                repo.setUpdate();
            }
        }
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