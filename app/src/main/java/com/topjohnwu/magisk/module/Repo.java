package com.topjohnwu.magisk.module;

import android.content.ContentValues;
import android.database.Cursor;

import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.WebService;

import java.util.Date;

public class Repo extends BaseModule {

    private static final String FILE_URL = "https://raw.githubusercontent.com/Magisk-Modules-Repo/%s/master/%s";
    private static final String ZIP_URL = "https://github.com/Magisk-Modules-Repo/%s/archive/master.zip";

    private String repoName;
    private Date mLastUpdate;

    public Repo(String name, Date lastUpdate) throws CacheModException {
        mLastUpdate = lastUpdate;
        repoName = name;
        update();
    }

    public Repo(Cursor c) {
        mId = c.getString(c.getColumnIndex("id"));
        mName = c.getString(c.getColumnIndex("name"));
        mVersion = c.getString(c.getColumnIndex("version"));
        mVersionCode = c.getInt(c.getColumnIndex("versionCode"));
        mAuthor = c.getString(c.getColumnIndex("author"));
        mDescription = c.getString(c.getColumnIndex("description"));
        repoName = c.getString(c.getColumnIndex("repo_name"));
        mLastUpdate = new Date(c.getLong(c.getColumnIndex("last_update")));
    }

    public void update() throws CacheModException {
        Logger.dev("Repo: Re-fetch prop");
        String props = WebService.request(getManifestUrl(), WebService.GET, true);
        String lines[] = props.split("\\n");
        parseProps(lines);
    }

    public void update(Date lastUpdate) throws CacheModException {
        Logger.dev("Repo: Local: " + mLastUpdate + " Remote: " + lastUpdate);
        if (mIsCacheModule)
            throw new CacheModException(mId);
        if (lastUpdate.after(mLastUpdate)) {
            mLastUpdate = lastUpdate;
            update();
        }
    }

    public ContentValues getContentValues() {
        ContentValues values = new ContentValues();
        values.put("id", mId);
        values.put("name", mName);
        values.put("version", mVersion);
        values.put("versionCode", mVersionCode);
        values.put("author", mAuthor);
        values.put("description", mDescription);
        values.put("repo_name", repoName);
        values.put("last_update", mLastUpdate.getTime());
        return values;
    }

    public String getZipUrl() {
        return String.format(ZIP_URL, repoName);
    }

    public String getLogUrl() {
        return String.format(FILE_URL, repoName, "changelog.txt");
    }

    public String getManifestUrl() {
        return String.format(FILE_URL, repoName, "module.prop");
    }

    public Date getLastUpdate() {
        return mLastUpdate;
    }
}
