package com.topjohnwu.magisk.module;

import android.util.Log;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;

public class Module {

    private String mRemoveFile;
    private String mDisableFile;

    private String mName = null;
    private String mVersion = "(No version provided)";
    private String mDescription = "(No description provided)";

    private boolean mEnable;
    private boolean mRemove;

    private String mId;
    private int mVersionCode;


    public Module(String path) {
        mRemoveFile = path + "/remove";
        mDisableFile = path + "/disable";
        for (String line : Utils.readFile(path + "/module.prop")) {
            String[] parts = line.split("=", 2);
            if (parts.length != 2) {
                continue;
            }

            String key = parts[0].trim();
            if (key.charAt(0) == '#') {
                continue;
            }

            String value = parts[1].trim();
            switch (key) {
                case "name":
                    mName = value;
                    break;
                case "version":
                    mVersion = value;
                    break;
                case "description":
                    mDescription = value;
                    break;
                case "id":
                    mId = value;
                    break;
                case "versionCode":
                    try {
                        mVersionCode = Integer.parseInt(value);
                    } catch (NumberFormatException e) {
                        mVersionCode = 0;
                    }
                    break;
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

}