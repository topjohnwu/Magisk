package com.topjohnwu.magisk.container;

import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.io.SuFile;

public class Module extends BaseModule {

    private SuFile mRemoveFile, mDisableFile, mUpdateFile;
    private boolean mEnable, mRemove, mUpdated;

    public Module(String path) {

        try {
            parseProps(Shell.Sync.su("dos2unix <  " + path + "/module.prop"));
        } catch (NumberFormatException ignored) {}

        mRemoveFile = new SuFile(path + "/remove");
        mDisableFile = new SuFile(path + "/disable");
        mUpdateFile = new SuFile(path + "/update");

        if (getId() == null) {
            int sep = path.lastIndexOf('/');
            setId(path.substring(sep + 1));
        }

        if (getName() == null) {
            setName(getId());
        }

        mEnable = !mDisableFile.exists();
        mRemove = mRemoveFile.exists();
        mUpdated = mUpdateFile.exists();
    }

    public void createDisableFile() {
        mEnable = false;
        mDisableFile.createNewFile();
    }

    public void removeDisableFile() {
        mEnable = true;
        mDisableFile.delete();
    }

    public boolean isEnabled() {
        return mEnable;
    }

    public void createRemoveFile() {
        mRemove = true;
        mRemoveFile.createNewFile();
    }

    public void deleteRemoveFile() {
        mRemove = false;
        mRemoveFile.delete();
    }

    public boolean willBeRemoved() {
        return mRemove;
    }

    public boolean isUpdated() {
        return mUpdated;
    }

}