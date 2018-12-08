package com.topjohnwu.magisk.container;

import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.io.SuFile;

public class Module extends BaseModule {

    private SuFile mRemoveFile, mDisableFile, mUpdateFile;
    private boolean mEnable, mRemove, mUpdated;

    public Module(String path) {

        try {
            parseProps(Shell.su("dos2unix <  " + path + "/module.prop").exec().getOut());
        } catch (NumberFormatException ignored) {}

        mRemoveFile = new SuFile(path, "remove");
        mDisableFile = new SuFile(path, "disable");
        mUpdateFile = new SuFile(path, "update");

        if (getId().isEmpty()) {
            int sep = path.lastIndexOf('/');
            setId(path.substring(sep + 1));
        }

        if (getName().isEmpty()) {
            setName(getId());
        }

        mEnable = !mDisableFile.exists();
        mRemove = mRemoveFile.exists();
        mUpdated = mUpdateFile.exists();
    }

    public void createDisableFile() {
        mEnable = !mDisableFile.createNewFile();
    }

    public void removeDisableFile() {
        mEnable = mDisableFile.delete();
    }

    public boolean isEnabled() {
        return mEnable;
    }

    public void createRemoveFile() {
        mRemove = mRemoveFile.createNewFile();
    }

    public void deleteRemoveFile() {
        mRemove = !mRemoveFile.delete();
    }

    public boolean willBeRemoved() {
        return mRemove;
    }

    public boolean isUpdated() {
        return mUpdated;
    }

}