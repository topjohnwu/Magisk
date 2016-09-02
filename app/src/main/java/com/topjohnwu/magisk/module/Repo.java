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
    public String name;
    public String baseUrl, zipUrl, manifestUrl, logUrl, manifest, version, moduleName, moduleDescription, moduleAuthor, moduleAuthorUrl;
    public Date lastUpdate;
    public Boolean usesRoot, usesXposed;
    private Context appContext;
    private SharedPreferences prefs;


    public Repo(String name, String url, Date updated, Context context) {
        appContext = context;
        this.name = name;
        this.baseUrl = url;
        this.lastUpdate = updated;
        this.fetch();

    }

    public Repo(String moduleName, String moduleDescription, String zipUrl, Date lastUpdated, Context context) {
        Log.d("Magisk", "Hey, I'm a repo!  My name is " + moduleName);
        appContext = context;
        this.zipUrl = zipUrl;
        this.moduleDescription = moduleDescription;
        this.moduleName = moduleName;
        this.lastUpdate = lastUpdated;


    }

    public void fetch() {
        prefs = PreferenceManager.getDefaultSharedPreferences(appContext);
        WebRequest webreq = new WebRequest();
        // Construct initial url for contents
        Log.d("Magisk", "Manifest string is: " + baseUrl + "/contents/");
        String repoString = webreq.makeWebServiceCall(baseUrl + "/contents/", WebRequest.GET);

        try {
            JSONArray repoArray = new JSONArray(repoString);

            for (int f = 0; f < repoArray.length(); f++) {
                JSONObject jsonobject = repoArray.getJSONObject(f);
                String name = jsonobject.getString("name");
                if (name.contains(".zip")) {
                    this.zipUrl = jsonobject.getString("download_url");
                } else if (name.equals("module.json")) {
                    this.manifestUrl = jsonobject.getString("download_url");
                } else if (name.equals("Changelog.txt")) {
                    this.logUrl = jsonobject.getString("download_url");
                }
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }

        Log.d("Magisk", "Inner fetch: " + repoString);
        try {
            WebRequest jsonReq = new WebRequest();
            // Construct initial url for contents
            String manifestString = webreq.makeWebServiceCall(this.manifestUrl, WebRequest.GET);
            JSONObject manifestObject = new JSONObject(manifestString);
            Log.d("Magisk", "Object: " + manifestObject.toString());
            version = manifestObject.getString("versionCode");
            moduleName = manifestObject.getString("moduleName");
            moduleDescription = manifestObject.getString("moduleDescription");
            moduleAuthor = manifestObject.getString("moduleAuthor");
            usesRoot = Boolean.getBoolean(manifestObject.getString("usesRoot"));
            usesXposed = Boolean.getBoolean(manifestObject.getString("usesXposed"));
            SharedPreferences.Editor editor = prefs.edit();
            String prefsString = "[{\"moduleDescription\":\"" + moduleDescription + "\","
                    + "\"moduleName\":\"" + moduleName + "\","
                    + "\"moduleAuthor\":\"" + moduleAuthor + "\","
                    + "\"moduleAuthorUrl\":\"" + moduleAuthorUrl + "\","
                    + "\"usesRoot\":\"" + usesRoot + "\","
                    + "\"usesXposed\":\"" + usesXposed + "\","
                    + "\"zipUrl\":\"" + zipUrl + "\","
                    + "\"lastUpdate\":\"" + lastUpdate + "\","
                    + "\"logUrl\":\"" + logUrl + "\"}]";
            editor.putString("module_" + moduleName, prefsString);
            editor.putBoolean("hasCachedRepos", true);
            SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd_HHmmss");
            editor.putString("updated", sdf.toString());
            Log.d("Magisk", "Storing Preferences: " + prefsString);
            editor.apply();

        } catch (JSONException e) {
            e.printStackTrace();
        }

    }

    public String getName() {
        return moduleName;
    }

    public String getVersion() {
        return version;
    }

    public String getDescription() {
        return moduleDescription;
    }

    public String getZipUrl() {
        return zipUrl;
    }

    public String getLogUrl() {
        return logUrl;
    }


}