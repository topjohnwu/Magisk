package com.topjohnwu.magisk.container;

import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.io.SuFile;

import java.io.IOException;

public class Module extends BaseModule {

    private SuFile mRemoveFile, mDisableFile, mUpdateFile;
    private boolean mEnable, mRemove, mUpdated;

    public Module(String path) {

        try {
            parseProps(Shell.Sync.su("cat " + path + "/module.prop"));
        } catch (NumberFormatException ignored) {}

        mRemoveFile = new SuFile(path + "/remove", true);
        mDisableFile = new SuFile(path + "/disable", true);
        mUpdateFile = new SuFile(path + "/update", true);

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
        try {
            mDisableFile.createNewFile();
        } catch (IOException ignored) {}
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
        try {
            mRemoveFile.createNewFile();
        } catch (IOException ignored) {}
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