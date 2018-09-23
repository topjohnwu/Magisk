package com.topjohnwu.magisk.container;

import android.content.ContentValues;
import android.database.Cursor;
import android.text.TextUtils;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.utils.Download;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;
import com.topjohnwu.magisk.Data;

import java.text.DateFormat;
import java.util.Date;

public class Repo extends BaseModule {

    private String repoName;
    private Date mLastUpdate;

    public Repo(String name) {
        repoName = name;
    }

    public Repo(Cursor c) {
        super(c);
        repoName = c.getString(c.getColumnIndex("repo_name"));
        mLastUpdate = new Date(c.getLong(c.getColumnIndex("last_update")));
    }

    public void update() throws IllegalRepoException {
        String props[] = Utils.dos2unix(WebService.getString(getManifestUrl())).split("\\n");
        try {
            parseProps(props);
        } catch (NumberFormatException e) {
            throw new IllegalRepoException("Repo [" + repoName + "] parse error: " + e.getMessage());
        }

        if (TextUtils.isEmpty(getId())) {
            throw new IllegalRepoException("Repo [" + repoName + "] does not contain id");
        }
        if (getVersionCode() < 0) {
            throw new IllegalRepoException("Repo [" + repoName + "] does not contain versionCode");
        }
        if (getMinMagiskVersion() < Const.MIN_MODULE_VER()) {
            Logger.debug("Repo [" + repoName + "] is outdated");
        }
    }

    public void update(Date lastUpdate) throws IllegalRepoException {
        mLastUpdate = lastUpdate;
        update();
    }

    @Override
    public ContentValues getContentValues() {
        ContentValues values = super.getContentValues();
        values.put("repo_name", repoName);
        values.put("last_update", mLastUpdate.getTime());
        return values;
    }

    public String getRepoName() {
        return repoName;
    }

    public String getZipUrl() {
        if (Data.cnRepo)
            return String.format(Const.Url.ZIP_URL_CN, repoName);
        return String.format(Const.Url.ZIP_URL, repoName);
    }

    public String getManifestUrl() {
        if (Data.cnRepo)
            return String.format(Const.Url.FILE_URL_CN, repoName, "module.prop");
        return String.format(Const.Url.FILE_URL, repoName, "module.prop");
    }

    public String getDetailUrl() {
        if (Data.cnRepo)
            return String.format(Const.Url.FILE_URL_CN, repoName, "README.md");
        return String.format(Const.Url.FILE_URL, repoName, "README.md");
    }

    public String getLastUpdateString() {
        return DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.MEDIUM).format(mLastUpdate);
    }

    public Date getLastUpdate() {
        return mLastUpdate;
    }

    public String getDownloadFilename() {
        return Download.getLegalFilename(getName() + "-" + getVersion() + ".zip");
    }

    public class IllegalRepoException extends Exception {
        IllegalRepoException(String message) {
            super(message);
        }
    }
}
