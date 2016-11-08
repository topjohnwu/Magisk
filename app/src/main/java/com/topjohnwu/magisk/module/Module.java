package com.topjohnwu.magisk.module;

import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;

public class Module extends BaseModule {

    private String mRemoveFile, mDisableFile, mUpdateFile;
    private boolean mEnable, mRemove, mUpdated;

    public Module(String path) throws CacheModException {

        parseProps(Utils.readFile(path + "/module.prop"));

        mRemoveFile = path + "/remove";
        mDisableFile = path + "/disable";
        mUpdateFile = path + "/update";

        if (mId == null) {
            int sep = path.lastIndexOf('/');
            mId = path.substring(sep + 1);
        }

        if (mName == null)
            mName = mId;

        Logger.dev("Creating Module, id: " + mId);

        mEnable = !Utils.itemExist(mDisableFile);
        mRemove = Utils.itemExist(mRemoveFile);
        mUpdated = Utils.itemExist(mUpdateFile);
    }

    public void createDisableFile() {
        mEnable = !Utils.createFile(mDisableFile);
    }

    public void removeDisableFile() {
        mEnable = Utils.removeItem(mDisableFile);
    }

    public boolean isEnabled() {
        return mEnable;
    }

    public void createRemoveFile() {
        mRemove = Utils.createFile(mRemoveFile);
    }

    public void deleteRemoveFile() {
        mRemove = !Utils.removeItem(mRemoveFile);
    }

    public boolean willBeRemoved() {
        return mRemove;
    }

    public boolean isUpdated() {
        return mUpdated;
    }

}