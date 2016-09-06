package com.topjohnwu.magisk.module;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.util.Log;

import com.topjohnwu.magisk.utils.WebRequest;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.text.SimpleDateFormat;
import java.util.Date;

public class Repo {
    private String mBaseUrl;
    private String mZipUrl;
    private String mLogUrl;
    private String mManifestUrl;
    private String mVersion;
    private String mName;
    private String mDescription;
    private String mAuthor;
    public String mAuthorUrl;
    private String mId;
    private String mVersionCode;
    private String mSupportUrl;
    private String mDonateUrl;
    private Date lastUpdate;
    private Context appContext;
    private boolean mIsInstalled;

    public Repo(String manifestString, Context context) {
        ParseProps(manifestString);
        appContext = context;

    }

    public Repo(String name, String url, Date updated, Context context) {
        appContext = context;
        this.mName = name;
        this.mBaseUrl = url;
        this.lastUpdate = updated;
        this.fetch();

    }

    public Repo(String moduleName, String moduleDescription, String zipUrl, Date lastUpdated, Context context) {
        appContext = context;
        this.mZipUrl = zipUrl;
        this.mDescription = moduleDescription;
        this.mName = moduleName;
        this.lastUpdate = lastUpdated;
        this.fetch();

    }

    private void fetch() {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(appContext);
        if (prefs.contains("repo_" + this.mId)) {
            String repoString = prefs.getString("repo_" + this.mId,"");
            if (!repoString.equals("")) {
                ParseProps(repoString);
            }
        }
        if (prefs.contains("repo_isInstalled_" + this.mId)) {
            mIsInstalled = prefs.getBoolean("repo_isInstalled_" + this.mId,false);

        }

        WebRequest webreq = new WebRequest();
        // Construct initial url for contents
        Log.d("Magisk", "Manifest string is: " + mBaseUrl + "/contents/");
        String repoString = webreq.makeWebServiceCall(mBaseUrl + "/contents/", WebRequest.GET);
        try {
            JSONArray repoArray = new JSONArray(repoString);
            for (int f = 0; f < repoArray.length(); f++) {
                JSONObject jsonobject = repoArray.getJSONObject(f);
                String name = jsonobject.getString("name");
                if (name.contains(".zip")) {
                    this.mZipUrl = jsonobject.getString("download_url");
                } else if (name.equals("module.prop")) {
                    this.mManifestUrl = jsonobject.getString("download_url");
                } else if (name.equals("changelog.txt")) {
                    this.mLogUrl = jsonobject.getString("download_url");
                }
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }

        Log.d("Magisk", "Inner fetch: " + repoString);
        WebRequest propReq = new WebRequest();
        String manifestString = propReq.makeWebServiceCall(this.mManifestUrl,WebRequest.GET,true);
        if (ParseProps(manifestString)) {
            PutProps(manifestString);
        }

    }

    private void PutProps(String manifestString) {
        manifestString = manifestString + "zipUrl=" + mZipUrl + "\nbaseUrl=" + mBaseUrl + "\nlogUrl=" + mLogUrl + "\nmanifestUrl=" + mManifestUrl;
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(appContext);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putString("repo_" + mId, manifestString);
        editor.putBoolean("hasCachedRepos", true);
        editor.putString("updated_" + mId, this.lastUpdate.toString());
        Log.d("Magisk", "Storing Preferences: " + manifestString);
        editor.apply();
    }
    private boolean ParseProps(String string) {
        Log.d("Magisk","Repo: parseprops called for string " + string);
        if ((string.length() <= 1) | (!string.contains("id"))) {
            return false;
        } else {
            String lines[] = string.split("\\n");
            for (String line : lines) {
                if (line != "") {
                    String props[] = line.split("=");
                    Log.d("Magisk", "Repo: Split values are " + props[0] + " and " + props[1]);
                    switch (props[0]) {
                        case "versionCode":
                            this.mVersionCode = props[1];
                            break;
                        case "name":
                            this.mName = props[1];
                            break;
                        case "author":
                            this.mAuthor = props[1];
                            break;
                        case "id":
                            this.mId = props[1];
                            break;
                        case "version":
                            this.mVersion = props[1];
                            break;
                        case "description":
                            this.mDescription = props[1];
                            break;
                        case "donate":
                            this.mDonateUrl = props[1];
                            break;
                        case "support":
                            this.mSupportUrl = props[1];
                            break;
                        case "donateUrl":
                            this.mDonateUrl = props[1];
                            break;
                        case "zipUrl":
                            this.mZipUrl = props[1];
                            break;
                        case "baseUrl":
                            this.mBaseUrl = props[1];
                            break;
                        case "manifestUrl":
                            this.mManifestUrl = props[1];
                            break;
                        default:
                            Log.d("Magisk", "Manifest string not recognized: " + props[0]);
                            break;
                    }
                }

            }
            return this.mName != null;

        }
    }

    public String getStringProperty(String mValue) {
        switch (mValue) {
            case "author":
                return mAuthor;
            case "id":
                return mId;
            case "version":
                return mVersion;
            case "description":
                return mDescription;
            case "supportUrl":
                return mSupportUrl;
            case "donateUrl":
                return mDonateUrl;
            case "baseeUrl":
                return mBaseUrl;
            case "zipUrl":
                return mZipUrl;
            default:
                return null;
        }
    }

    public String getName() {
        return mName;
    }

    public String getmVersion() {
        return mVersion;
    }

    public int getmVersionCode() {
        return Integer.valueOf(mVersionCode);
    }

    public String getDescription() {
        return mDescription;
    }

    public String getId() {
        return mId;
    }

    public String getmZipUrl() {
        return mZipUrl;
    }

    public String getmBaseUrl() {
        return mBaseUrl;
    }

    public String getmLogUrl() {
        return mLogUrl;
    }


    public String getmAuthor() {
        return mAuthor;
    }

    public String getmDonateUrl() {
        return mDonateUrl;
    }

    public String getmManifestUrl() {
        return mManifestUrl;
    }

    public String getmSupportUrl() {
        return mSupportUrl;
    }

    public Date getLastUpdate() {
        return lastUpdate;
    }

    public boolean isInstalled() { return mIsInstalled; }
}

