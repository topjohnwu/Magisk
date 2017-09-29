package com.topjohnwu.magisk.container;

import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

public class Module extends BaseModule {

    private String mRemoveFile, mDisableFile, mUpdateFile;
    private boolean mEnable, mRemove, mUpdated;

    public Module(Shell shell, String path) throws CacheModException {

        parseProps(Utils.readFile(shell, path + "/module.prop"));

        mRemoveFile = path + "/remove";
        mDisableFile = path + "/disable";
        mUpdateFile = path + "/update";

        if (getId() == null) {
            int sep = path.lastIndexOf('/');
            setId(path.substring(sep + 1));
        }

        if (getName() == null) {
            setName(getId());
        }

        Logger.dev("Creating Module, id: " + getId());

        mEnable = !Utils.itemExist(shell, mDisableFile);
        mRemove = Utils.itemExist(shell, mRemoveFile);
        mUpdated = Utils.itemExist(shell, mUpdateFile);
    }

    public void createDisableFile(Shell shell) {
        mEnable = false;
        Utils.createFile(shell, mDisableFile);
    }

    public void removeDisableFile(Shell shell) {
        mEnable = true;
        Utils.removeItem(shell, mDisableFile);
    }

    public boolean isEnabled() {
        return mEnable;
    }

    public void createRemoveFile(Shell shell) {
        mRemove = true;
        Utils.createFile(shell, mRemoveFile);
    }

    public void deleteRemoveFile(Shell shell) {
        mRemove = false;
        Utils.removeItem(shell, mRemoveFile);
    }

    public boolean willBeRemoved() {
        return mRemove;
    }

    public boolean isUpdated() {
        return mUpdated;
    }

}