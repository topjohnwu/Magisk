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
    private String lastUpdate;
    private Context appContext;
    private boolean mIsInstalled,mCanUpdate;

    public Repo(String manifestString, Context context) {
        appContext = context;
        ParseProps(manifestString);

    }



    public Repo(String name, String url, Date updated, Context context) {
        appContext = context;
        this.mName = name;
        this.mBaseUrl = url;
        this.lastUpdate = updated.toString();
        this.fetch();

    }

    public Repo(String moduleName, String moduleDescription, String zipUrl, Date lastUpdated, Context context) {
        appContext = context;
        this.mZipUrl = zipUrl;
        this.mDescription = moduleDescription;
        this.mName = moduleName;
        this.lastUpdate = lastUpdated.toString();
        this.fetch();

    }

    public void fetch() {
        WebRequest webreq = new WebRequest();
        // Construct initial url for contents
        Log.d("Magisk", "Repo: Fetch called, Manifest string is: " + mBaseUrl + "/contents?access_token=5c9f47a299d48a6a649af3587bc97200bafcac65");
        String repoString = webreq.makeWebServiceCall(mBaseUrl + "/contents?access_token=5c9f47a299d48a6a649af3587bc97200bafcac65", WebRequest.GET);
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

        Log.d("Magisk", "Repo: Inner fetch: " + mManifestUrl + "?access_token=5c9f47a299d48a6a649af3587bc97200bafcac65");
        WebRequest propReq = new WebRequest();
        String manifestString = propReq.makeWebServiceCall(mManifestUrl,WebRequest.GET,true);
        Log.d("Magisk","Repo: parseprops called from fetch for string " + manifestString);

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

        if ((string.length() <= 1) | (!string.contains("id"))) {
            return false;
        } else {
            String lines[] = string.split("\\n");
            for (String line : lines) {
                if (line != "") {
                    String props[] = line.split("=");
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
                        case "logUrl":
                            this.mLogUrl = props[1];
                            break;
                        default:
                            Log.d("Magisk", "Manifest string not recognized: " + props[0]);
                            break;
                    }
                }

            }
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(appContext);
                if (prefs.contains("repo-isInstalled_" + this.mId)) {
                    mIsInstalled = prefs.getBoolean("repo-isInstalled_" + this.mId,false);
                }
                if (prefs.contains("repo-canUpdate_" + this.mId)) {
                    mCanUpdate = prefs.getBoolean("repo-canUpdate_" + this.mId,false);
                }
                if (prefs.contains("updated_" + this.mId)) {
                    lastUpdate = prefs.getString("updated_" + this.mId,"");
                }



            return this.mId != null;

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

    public String getLastUpdate() {
        return lastUpdate;
    }

    public boolean isInstalled() { return mIsInstalled; }
    public boolean canUpdate() { return mCanUpdate; }
}

