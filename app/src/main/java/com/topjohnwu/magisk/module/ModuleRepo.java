package com.topjohnwu.magisk.module;

import android.app.ListActivity;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;

import com.topjohnwu.magisk.utils.WebRequest;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

public class ModuleRepo {
    private String[] result;
    private static String url = "https://api.github.com/orgs/Magisk-Modules-Repo/repos";
    private static List<Repo> repos = new ArrayList<Repo>();
    private static final String TAG_ID = "id";
    private static final String TAG_NAME = "name";
    private Context activityContext;


    public List<Repo> listRepos() {
        MyAsyncTask asynchTask = new MyAsyncTask();
        Log.d("Magisk", "Gitagent init called");
        asynchTask.execute();
        List<String> out = null;



        return repos;
    }

    public JSONArray fetchModuleArray() {
        fetchRepoInfo();
        parseRepoInfo();
        JSONArray moduleArray = enumerateModules();
        return null;
    }

    private void fetchRepoInfo() {

    }

    private void parseRepoInfo() {

    }

    private JSONArray enumerateModules() {
        JSONArray enumeratedModules = new JSONArray();
        return enumeratedModules;
    }

    class MyAsyncTask extends AsyncTask<String, String, Void> {

        @Override
        protected void onPreExecute() {
            super.onPreExecute();


        }

        @Override
        protected Void doInBackground(String... params) {
            Log.d("Magisk", "doInBackground running");
            // Creating service handler class instance
            WebRequest webreq = new WebRequest();

            // Making a request to url and getting response
            String jsonStr = webreq.makeWebServiceCall(url, WebRequest.GET);

            Log.d("Response: ", "> " + jsonStr);

            try {
                repos.clear();
                JSONArray jsonArray = new JSONArray(jsonStr);
                for (int i = 0; i < jsonArray.length(); i++) {
                    JSONObject jsonobject = jsonArray.getJSONObject(i);
                    String name = jsonobject.getString("name");
                    String urlString = jsonobject.getString("html_url");
                    try {
                        URL url = new URL(urlString);
                        if (!name.contains("Repo.github.io")) {
                            repos.add(new Repo(name, url));
                        }
                    } catch (MalformedURLException e) {
                        e.printStackTrace();
                    }

                }
                for (int i = 0; i < repos.size(); i++) {
                    Repo repo = repos.get(i);
                    Log.d("Magisk", repo.name + " URL: " + repo.url);

                }
            } catch (JSONException e) {
                e.printStackTrace();
            }


            return null;


        }

        protected void onPostExecute(Void v) {


        } // protected void onPostExecute(Void v)
    } //class MyAsyncTask extends AsyncTask<String, String, Void>

    protected void onPreExecute() {

    }

    public class Repo {
        public String name;
        public URL url;
        public String manifest, version, moduleName, moduleDescription, moduleAuthor, moduleAuthorUrl;
        public Boolean usesRoot,usesXposed;


        public Repo(String name, URL url) {
            this.name = name;
            this.url = url;
            this.manifest = ("https://raw.githubusercontent.com/Magisk-Modules-Repo/" + name + "/master/module.json");
            WebRequest webreq = new WebRequest();

            // Making a request to url and getting response
            String manifestString = webreq.makeWebServiceCall(manifest, WebRequest.GET);
            Log.d("Magisk","Inner fetch: " + manifestString);
            try {
                JSONObject jsonobject = new JSONObject(manifestString);
                Log.d("Magisk","Object: " +jsonobject.toString());
                version = jsonobject.getString("versionCode");
                moduleName = jsonobject.getString("moduleName");
                moduleDescription = jsonobject.getString("moduleDescription");
                moduleAuthor = jsonobject.getString("moduleAuthor");
                moduleAuthorUrl = jsonobject.getString("authorUrl");
                usesRoot = Boolean.getBoolean(jsonobject.getString("usesRoot"));
                usesXposed = Boolean.getBoolean(jsonobject.getString("usesXposed"));

            } catch (JSONException e) {
                e.printStackTrace();
            }

            Log.d("Magisk","We're in! " + " " + version + " " + moduleName + " " + moduleAuthor + " " + moduleDescription + " " + moduleAuthorUrl);

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
    }

}
