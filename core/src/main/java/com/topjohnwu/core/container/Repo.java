package com.topjohnwu.core.container;

import android.content.ContentValues;
import android.database.Cursor;
import android.os.Parcel;
import android.os.Parcelable;

import com.topjohnwu.core.Const;
import com.topjohnwu.core.utils.Logger;
import com.topjohnwu.core.utils.Utils;

import java.text.DateFormat;
import java.util.Date;

public class Repo extends BaseModule {

    private Date mLastUpdate;

    public Repo(String id) {
        setId(id);
    }

    public Repo(Cursor c) {
        super(c);
        mLastUpdate = new Date(c.getLong(c.getColumnIndex("last_update")));
    }

    public Repo(Parcel p) {
        super(p);
        mLastUpdate = new Date(p.readLong());
    }

    public static final Parcelable.Creator<Repo> CREATOR = new Parcelable.Creator<Repo>() {

        @Override
        public Repo createFromParcel(Parcel source) {
            return new Repo(source);
        }

        @Override
        public Repo[] newArray(int size) {
            return new Repo[size];
        }
    };

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        super.writeToParcel(dest, flags);
        dest.writeLong(mLastUpdate.getTime());
    }

    public void update() throws IllegalRepoException {
        String props[] = Utils.dlString(getPropUrl()).split("\\n");
        try {
            parseProps(props);
        } catch (NumberFormatException e) {
            throw new IllegalRepoException("Repo [" + getId() + "] parse error: " + e.getMessage());
        }

        if (getVersionCode() < 0) {
            throw new IllegalRepoException("Repo [" + getId() + "] does not contain versionCode");
        }
        if (getMinMagiskVersion() < Const.MIN_MODULE_VER) {
            Logger.debug("Repo [" + getId() + "] is outdated");
        }
    }

    public void update(Date lastUpdate) throws IllegalRepoException {
        mLastUpdate = lastUpdate;
        update();
    }

    @Override
    public ContentValues getContentValues() {
        ContentValues values = super.getContentValues();
        values.put("last_update", mLastUpdate.getTime());
        return values;
    }

    public String getZipUrl() {
        return String.format(Const.Url.ZIP_URL, getId());
    }

    public String getPropUrl() {
        return String.format(Const.Url.FILE_URL, getId(), "module.prop");
    }

    public String getDetailUrl() {
        return String.format(Const.Url.FILE_URL, getId(), "README.md");
    }

    public String getLastUpdateString() {
        return DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.MEDIUM).format(mLastUpdate);
    }

    public Date getLastUpdate() {
        return mLastUpdate;
    }

    public String getDownloadFilename() {
        return Utils.getLegalFilename(getName() + "-" + getVersion() + ".zip");
    }

    public class IllegalRepoException extends Exception {
        IllegalRepoException(String message) {
            super(message);
        }
    }
}
