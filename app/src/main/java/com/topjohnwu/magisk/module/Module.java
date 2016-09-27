package com.topjohnwu.magisk.module;

import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.RepoHelper;
import com.topjohnwu.magisk.utils.Utils;

public class Module extends BaseModule {

    private String mRemoveFile;
    private String mDisableFile;
    private boolean mEnable, mRemove, mUpdateAvailable = false;

    public Module(String path) {

        parseProps(Utils.readFile(path + "/module.prop"));

        mRemoveFile = path + "/remove";
        mDisableFile = path + "/disable";

        if (mId == null) {
            int sep = path.lastIndexOf('/');
            mId = path.substring(sep + 1);
        }

        if (mName == null)
            mName = mId;

        Logger.dh("Creating Module, id: " + mId);

        mEnable = !Utils.itemExist(mDisableFile);
        mRemove = Utils.itemExist(mRemoveFile);

    }

    public void checkUpdate() {
        Repo repo = RepoHelper.repoMap.get(mId);
        if (repo != null) {
            repo.setInstalled();
            if (repo.getVersionCode() > mVersionCode) {
                repo.setUpdate();
                mUpdateAvailable = true;
            }
        }
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

    public boolean isUpdateAvailable() { return mUpdateAvailable; }

}