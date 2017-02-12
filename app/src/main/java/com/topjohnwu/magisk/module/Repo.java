package com.topjohnwu.magisk.module;

import android.content.Context;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.WebService;

import java.util.Date;

public class Repo extends BaseModule {
    private String mLogUrl, mManifestUrl, mZipUrl;
    private Date mLastUpdate;

    public Repo(Context context, String name, Date lastUpdate) throws CacheModException {
        mLastUpdate = lastUpdate;
        mLogUrl = context.getString(R.string.file_url, name, "changelog.txt");
        mManifestUrl = context.getString(R.string.file_url, name, "module.prop");
        mZipUrl = context.getString(R.string.zip_url, name);
        update();
    }

    public void update() throws CacheModException {
        Logger.dev("Repo: Re-fetch prop");
        String props = WebService.request(mManifestUrl, WebService.GET, true);
        String lines[] = props.split("\\n");
        parseProps(lines);
    }

    public void update(Date lastUpdate) throws CacheModException {
        Logger.dev("Repo: Old: " + mLastUpdate + " New: " + lastUpdate);
        if (mIsCacheModule)
            throw new CacheModException(mId);
        if (lastUpdate.after(mLastUpdate)) {
            mLastUpdate = lastUpdate;
            update();
        }
    }

    public String getZipUrl() {
        return mZipUrl;
    }

    public String getLogUrl() {
        return mLogUrl;
    }

    public String getManifestUrl() {
        return mManifestUrl;
    }

    public Date getLastUpdate() {
        return mLastUpdate;
    }
}
