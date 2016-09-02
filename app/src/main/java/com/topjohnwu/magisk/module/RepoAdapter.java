package com.topjohnwu.magisk.module;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.preference.PreferenceManager;
import android.util.Log;
import android.widget.Toast;

import com.topjohnwu.magisk.utils.WebRequest;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Map;

public class RepoAdapter {
    private String[] result;
    private static String url = "https://api.github.com/orgs/Magisk-Modules-Repo/repos";
    private static List<Repo> repos = new ArrayList<Repo>();
    private static final String TAG_ID = "id";
    private static final String TAG_NAME = "name";
    private Context activityContext;
    private Date updatedDate, currentDate;

    public List<Repo> listRepos(Context context, boolean refresh) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        if (!prefs.contains("hasCachedRepos") | refresh) {
            activityContext = context;
            new MyAsyncTask().execute();
            List<String> out = null;
        } else {
            Log.d("Magisk", "Building from cache");
            Map<String, ?> map = prefs.getAll();
            repos.clear();
            for (Map.Entry<String, ?> entry : map.entrySet()) {
                if (entry.getKey().contains("module_")) {
                    String repoString = entry.getValue().toString().replace("&quot;", "\"");
                    JSONArray repoArray = null;
                    try {
                        repoArray = new JSONArray(repoString);


                        for (int f = 0; f < repoArray.length(); f++) {
                            JSONObject jsonobject = repoArray.getJSONObject(f);
                            String name = entry.getKey().replace("module_", "");
                            name = name.replace(" ", "");
                            String moduleName, moduleDescription, zipUrl;
                            moduleName = jsonobject.getString("moduleName");
                            moduleDescription = jsonobject.getString("moduleDescription");
                            zipUrl = jsonobject.getString("zipUrl");
                            SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'");
                            try {
                                updatedDate = format.parse(jsonobject.getString("lastUpdate"));
                            } catch (ParseException e) {
                                e.printStackTrace();
                            }
                            repos.add(new Repo(name, moduleDescription, zipUrl, updatedDate, activityContext));

                        }
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }
                }

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
        protected void onProgressUpdate(String... values) {
            super.onProgressUpdate(values);
            Toast.makeText(activityContext, "Refreshing online modules", Toast.LENGTH_SHORT).show();

        }

        @Override
        protected Void doInBackground(String... params) {
            publishProgress();
            // Creating service handler class instance
            WebRequest webreq = new WebRequest();

            // Making a request to url and getting response
            String jsonStr = webreq.makeWebServiceCall(url, WebRequest.GET);
            Log.d("Magisk", "doInBackground Running, String: " + jsonStr + " Url: " + url);


            try {
                repos.clear();
                JSONArray jsonArray = new JSONArray(jsonStr);
                for (int i = 0; i < jsonArray.length(); i++) {
                    JSONObject jsonobject = jsonArray.getJSONObject(i);
                    String name = jsonobject.getString("name");
                    String url = jsonobject.getString("url");
                    String lastUpdate = jsonobject.getString("updated_at");
                    SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'");
                    try {
                        updatedDate = format.parse(lastUpdate);

                    } catch (ParseException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }

                    if (!name.contains("Repo.github.io")) {
                        repos.add(new Repo(name, url, updatedDate, activityContext));
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
