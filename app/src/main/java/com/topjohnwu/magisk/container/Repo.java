package com.topjohnwu.magisk.container;

import android.content.ContentValues;
import android.database.Cursor;

import com.topjohnwu.magisk.utils.WebService;

import java.util.Date;

public class Repo extends BaseModule {

    private static final String FILE_URL = "https://raw.githubusercontent.com/Magisk-Modules-Repo/%s/master/%s";
    private static final String ZIP_URL = "https://github.com/Magisk-Modules-Repo/%s/archive/master.zip";

    private String repoName;
    private Date mLastUpdate;

    public Repo(String name, Date lastUpdate) {
        mLastUpdate = lastUpdate;
        repoName = name;
        update();
    }

    public Repo(Cursor c) {
        super(c);
        repoName = c.getString(c.getColumnIndex("repo_name"));
        mLastUpdate = new Date(c.getLong(c.getColumnIndex("last_update")));
    }

    public void update() {
        String props = WebService.getString(getManifestUrl());
        String lines[] = props.split("\\n");
        parseProps(lines);
    }

    public boolean update(Date lastUpdate) {
        if (lastUpdate.after(mLastUpdate)) {
            mLastUpdate = lastUpdate;
            update();
            return true;
        }
        return false;
    }

    public ContentValues getContentValues() {
        ContentValues values = super.getContentValues();
        values.put("repo_name", repoName);
        values.put("last_update", mLastUpdate.getTime());
        return values;
    }

    public String getZipUrl() {
        return String.format(ZIP_URL, repoName);
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
