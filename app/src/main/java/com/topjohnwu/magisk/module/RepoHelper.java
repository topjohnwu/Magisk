package com.topjohnwu.magisk.module;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.preference.PreferenceManager;
import android.util.Log;
import android.widget.Toast;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebRequest;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Map;

public class RepoHelper {
    private static List<Repo> repos = new ArrayList<>();
    private static String TAG = "Magisk";
    private Context activityContext;
    private Date updatedDate;
    private SharedPreferences prefs;
    private boolean apiFail;

    public RepoHelper() {
    }

    public List<Repo> listRepos(Context context, boolean refresh, TaskDelegate delegate) {
        prefs = PreferenceManager.getDefaultSharedPreferences(context);
        activityContext = context;

        if (!prefs.contains("hasCachedRepos") | refresh) {
            Log.d(TAG, "RepoHelper: Building from web");
            new BuildFromWeb(delegate).execute();
            SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
            String date = format.format(Calendar.getInstance().getTime());
            prefs.edit().putString("last_update",date).apply();
        } else {
            Log.d(TAG, "RepoHelper: Building from cache");
            BuildFromCache();
        }

        Collections.sort(repos, new CustomComparator());
        return repos;
    }

    private void BuildFromCache() {
        repos.clear();
        Map<String, ?> map = prefs.getAll();
        for (Map.Entry<String, ?> entry : map.entrySet()) {
            if (entry.getKey().contains("repo_")) {
                String repoString = entry.getValue().toString().replace("&quot;", "\"");
                repos.add(new Repo(repoString, activityContext));
            }
        }
    }

    class BuildFromWeb extends AsyncTask<String, String, Void> {

        private TaskDelegate delegate;

        public BuildFromWeb(TaskDelegate delegate) {
            this.delegate = delegate;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

        }

        @Override
        protected void onProgressUpdate(String... values) {
            super.onProgressUpdate(values);
            prefs.edit().putBoolean("ignoreUpdateAlerts", false).apply();
            Toast.makeText(activityContext, "Refreshing online modules", Toast.LENGTH_SHORT).show();

        }

        @Override
        protected Void doInBackground(String... params) {
            publishProgress();
            // Creating service handler class instance
            WebRequest webreq = new WebRequest();

            // Making a request to url and getting response
            String token = activityContext.getString(R.string.some_string);
            String url1 = activityContext.getString(R.string.url_main);
            String jsonStr = webreq.makeWebServiceCall(url1 + Utils.procFile(token, activityContext), WebRequest.GET);
            if (jsonStr != null && !jsonStr.isEmpty()) {

                try {
                    repos.clear();
                    JSONArray jsonArray = new JSONArray(jsonStr);
                    for (int i = 0; i < jsonArray.length(); i++) {
                        JSONObject jsonobject = jsonArray.getJSONObject(i);
                        String name = jsonobject.getString("name");
                        String url = jsonobject.getString("url");
                        String lastUpdate = jsonobject.getString("updated_at");
                        String mId = "";
                        String cacheUpdate = "";
                        String manifestString = "";
                        boolean doUpdate = true;
                        boolean hasCachedDate = false;
                        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
                        Map<String, ?> map = prefs.getAll();
                        for (Map.Entry<String, ?> entry : map.entrySet()) {
                            if (entry.getValue().toString().contains(url)) {
                                Log.d("Magisk", "RepoHelper: found matching URL");
                                manifestString = entry.getValue().toString();
                                String[] entryStrings = entry.getValue().toString().split("\n");
                                for (String valueString : entryStrings) {
                                    String[] valueSub = valueString.split("=");
                                    if (valueSub[0].equals("id")) {
                                        mId = valueSub[1];
                                        if (prefs.contains("updated_" + mId)) {
                                            cacheUpdate = prefs.getString("updated_" + mId, "");
                                            hasCachedDate = true;
                                        }
                                    }
                                }

                            }
                        }
                        try {
                            updatedDate = format.parse(lastUpdate);
                            Log.d("Magisk", "RepoHelper: Dates found, online is " + updatedDate + " and cached is " + cacheUpdate);

                            if (hasCachedDate) {

                                doUpdate = !cacheUpdate.equals(updatedDate.toString());
                                Log.d("Magisk", "RepoHelper: DoUpdate is " + doUpdate);
                            }

                        } catch (ParseException e) {
                            // TODO Auto-generated catch block
                            e.printStackTrace();
                        }
                        if (!name.contains("Repo.github.io")) {
                            if (doUpdate) {
                                repos.add(new Repo(name, url, updatedDate, activityContext));
                                Log.d(TAG, "RepoHelper: Trying to add new repo for online refresh - " + name);
                            } else {
                                repos.add(new Repo(manifestString, activityContext));
                                Log.d(TAG, "RepoHelper: Trying to add new repo using manifestString of " + mId);
                            }
                        }
                        for (int f = 0; f < repos.size(); f++) {
                            repos.get(f).fetch();
                        }
                    }
                } catch (JSONException e) {
                    e.printStackTrace();
                }
                apiFail = false;
            } else {
                apiFail = true;
            }
            return null;

        }

        protected void onPostExecute(Void v) {
            if (apiFail) {
                Toast.makeText(activityContext, "GitHub API Limit reached, please try refreshing again in an hour.", Toast.LENGTH_LONG).show();
            } else {
                Log.d("Magisk", "RepoHelper: postExecute fired");
                delegate.taskCompletionResult("Complete");
                BuildFromCache();

            }

        }
    }

    public interface TaskDelegate {
        void taskCompletionResult(String result);
    }

    public class CustomComparator implements Comparator<Repo> {
        @Override
        public int compare(Repo o1, Repo o2) {
            return o1.getName().compareTo(o2.getName());
        }
    }

}
