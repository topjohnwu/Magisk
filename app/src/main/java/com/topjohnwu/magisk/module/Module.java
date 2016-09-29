package com.topjohnwu.magisk.module;

import android.os.AsyncTask;

import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;

public class Module extends BaseModule {

    private String mRemoveFile, mDisableFile, mUpdateFile;
    private boolean mEnable, mRemove, mUpdated;

    public Module(String path) {

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

        try {
            mEnable = !Utils.itemExist(mDisableFile);
        } catch (Exception e) {
            mEnable = false;
        }
        try {
            mRemove = Utils.itemExist(mRemoveFile);
        } catch (Exception e) {
            mRemove = false;
        }
        try {
            mUpdated = Utils.itemExist(mUpdateFile);
        } catch (Exception e) {
            mUpdated = false;
        }

    }

    public void createDisableFile() {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                mEnable = !Utils.createFile(mDisableFile);
                return null;
            }
        }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
    }

    public void removeDisableFile() {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                mEnable = Utils.removeFile(mDisableFile);
                return null;
            }
        }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
    }

    public boolean isEnabled() {
        return mEnable;
    }

    public void createRemoveFile() {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                mRemove = Utils.createFile(mRemoveFile);
                return null;
            }
        }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
    }

    public void deleteRemoveFile() {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                mRemove = !Utils.removeFile(mRemoveFile);
                return null;
            }
        }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
    }

    public boolean willBeRemoved() {
        return mRemove;
    }

    public boolean isUpdated() {
        return mUpdated;
    }

}