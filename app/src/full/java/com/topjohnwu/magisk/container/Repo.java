package com.topjohnwu.magisk.container;

import android.content.ContentValues;
import android.database.Cursor;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;

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

        if (getId() == null) {
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
        return String.format(Const.Url.ZIP_URL, repoName);
    }

    public String getManifestUrl() {
        return String.format(Const.Url.FILE_URL, repoName, "module.prop");
    }

    public String getDetailUrl() {
        return String.format(Const.Url.FILE_URL, repoName, "README.md");
    }

    public String getLastUpdateString() {
        return DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.MEDIUM,
                MagiskManager.locale).format(mLastUpdate);
    }

    public Date getLastUpdate() {
        return mLastUpdate;
    }

    public class IllegalRepoException extends Exception {
        IllegalRepoException(String message) {
            super(message);
        }
    }
}
