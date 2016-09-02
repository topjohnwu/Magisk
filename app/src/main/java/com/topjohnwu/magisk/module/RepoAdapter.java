package com.topjohnwu.magisk.module;

import android.app.ListActivity;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;

import com.topjohnwu.magisk.utils.WebRequest;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class RepoAdapter {
    private String[] result;
    private static String url = "https://api.github.com/orgs/Magisk-Modules-Repo/repos";
    private static List<Repo> repos = new ArrayList<Repo>();
    private static final String TAG_ID = "id";
    private static final String TAG_NAME = "name";
    private Context activityContext;


    public List<Repo> listRepos(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        if (!prefs.contains("hasCachedRepos")) {
            activityContext = context;
            MyAsyncTask asynchTask = new MyAsyncTask();
            asynchTask.execute();
            List<String> out = null;
        } else {
            Log.d("Magisk", "Building from cache");
            Map<String,?> map = prefs.getAll();
            for (Map.Entry<String,?> entry : map.entrySet())
            {
                if (entry.getKey().contains("module_")) {
                    String repoString = entry.getValue().toString();
                    JSONArray repoArray = null;
                    try {
                        repoArray = new JSONArray(repoString);

                    repos.clear();
                    for (int f = 0; f < repoArray.length(); f++) {
                        JSONObject jsonobject = repoArray.getJSONObject(f);
                        String name = jsonobject.getString("name");
                        String moduleName, moduleDescription, zipUrl;
                        moduleName = jsonobject.getString("moduleName");
                        moduleDescription = jsonobject.getString("moduleDescription");
                        zipUrl = jsonobject.getString("zipUrl");
                        repos.add(new Repo(moduleName,moduleDescription,zipUrl));

                    }
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }
                }
                System.out.println(entry.getKey() + "/" + entry.getValue());
            }

        }


        return repos;
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
                    String url = jsonobject.getString("url");
                        if (!name.contains("Repo.github.io")) {
                            repos.add(new Repo(name, url, activityContext));
                        }
                    for (int f = 0; f < repos.size(); f++) {
                        repos.get(f).fetch();
                    }
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



}
