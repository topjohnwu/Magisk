package com.topjohnwu.magisk.module;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.ModuleHelper;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebRequest;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.Date;

public class Repo extends BaseModule {
    protected String repoName, mLogUrl, mManifestUrl, mZipUrl;
    protected Date mLastUpdate;

    public Repo(Context context, String name, Date lastUpdate) {
        repoName = name;
        mLastUpdate = lastUpdate;
        mLogUrl = context.getString(R.string.file_url, repoName, "changelog.txt");
        mManifestUrl = context.getString(R.string.file_url, repoName, "module.prop");
        mZipUrl = context.getString(R.string.zip_url, repoName);
        update();
    }

    public void update() {
        Logger.dev("Repo: Re-fetch prop " + mId);
        String props = WebRequest.makeWebServiceCall(mManifestUrl, WebRequest.GET, true);
        String lines[] = props.split("\\n");
        parseProps(lines);
    }

    public void update(Date lastUpdate) {
        Logger.dev("Repo: Old: " + mLastUpdate);
        Logger.dev("Repo: New: " + lastUpdate);
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
