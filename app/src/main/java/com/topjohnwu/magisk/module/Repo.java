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
        super(c);
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
        if (lastUpdate.after(mLastUpdate)) {
            mLastUpdate = lastUpdate;
            update();
        }
    }

    public ContentValues getContentValues() {
        ContentValues values = new ContentValues();
        values.put("id", getId());
        values.put("name", getName());
        values.put("version", getVersion());
        values.put("versionCode", getVersionCode());
        values.put("author", getAuthor());
        values.put("description", getDescription());
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

    public String getDetailUrl() {
        return String.format(FILE_URL, repoName, "README.md");
    }

    public Date getLastUpdate() {
        return mLastUpdate;
    }
}
