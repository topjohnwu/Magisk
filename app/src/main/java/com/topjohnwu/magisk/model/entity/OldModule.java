package com.topjohnwu.magisk.model.entity;

import android.os.Parcel;
import android.os.Parcelable;

import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.io.SuFile;

public class OldModule extends BaseModule {

    public static final Parcelable.Creator<OldModule> CREATOR = new Creator<OldModule>() {
        /* It won't be used at any place */
        @Override
        public OldModule createFromParcel(Parcel source) {
            return null;
        }

        @Override
        public OldModule[] newArray(int size) {
            return null;
        }
    };
    private final SuFile mRemoveFile;
    private final SuFile mDisableFile;
    private final SuFile mUpdateFile;
    private final boolean mUpdated;
    private boolean mEnable;
    private boolean mRemove;

    public OldModule(String path) {

        try {
            parseProps(Shell.su("dos2unix <  " + path + "/module.prop").exec().getOut());
        } catch (NumberFormatException ignored) {
        }

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
