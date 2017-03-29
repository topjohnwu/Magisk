package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.content.SharedPreferences;
import android.text.TextUtils;

import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.module.BaseModule;
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.ValueSortedMap;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

public class LoadRepos extends ParallelTask<Void, Void, Void> {

    public static final String ETAG_KEY = "ETag";

    private static final String REPO_URL = "https://api.github.com/orgs/Magisk-Modules-Repo/repos";

    private String prefsPath;

    public LoadRepos(Activity context) {
        super(context);
        prefsPath = context.getApplicationInfo().dataDir + "/shared_prefs";
    }

    @Override
    protected Void doInBackground(Void... voids) {
        Logger.dev("LoadRepos: Loading repos");

        SharedPreferences prefs = magiskManager.prefs;

        RepoDatabaseHelper dbHelper = new RepoDatabaseHelper(magiskManager);

        // Legacy data cleanup
        File old = new File(prefsPath, "RepoMap.xml");
        if (old.exists() || !prefs.getString("repomap", "empty").equals("empty")) {
            old.delete();
            prefs.edit().remove("version").remove("repomap").remove(ETAG_KEY).apply();
            dbHelper.clearRepo();
        }

        Map<String, String> header = new HashMap<>();
        // Get cached ETag to add in the request header
        String etag = prefs.getString(ETAG_KEY, "");
        header.put("If-None-Match", etag);

        // Make a request to main URL for repo info
        String jsonString = WebService.request(REPO_URL, WebService.GET, null, header, false);

        ValueSortedMap<String, Repo> cached = dbHelper.getRepoMap(false), fetched = new ValueSortedMap<>();

        if (!TextUtils.isEmpty(jsonString)) {
            try {
                JSONArray jsonArray = new JSONArray(jsonString);
                // If it gets to this point, the response is valid, update ETag
                etag = WebService.getLastResponseHeader().get(ETAG_KEY).get(0);
                // Maybe bug in Android build tools, sometimes the ETag has crap in it...
                etag = etag.substring(etag.indexOf('\"'), etag.lastIndexOf('\"') + 1);

                // Update repo info
                for (int i = 0; i < jsonArray.length(); i++) {
                    JSONObject jsonobject = jsonArray.getJSONObject(i);
                    String id = jsonobject.getString("description");
                    String name = jsonobject.getString("name");
                    String lastUpdate = jsonobject.getString("pushed_at");
                    SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
                    Date updatedDate;
                    try {
                        updatedDate = format.parse(lastUpdate);
                    } catch (ParseException e) {
                        continue;
                    }
                    Repo repo = cached.get(id);
                    try {
                        if (repo == null) {
                            Logger.dev("LoadRepos: Create new repo " + id);
                            repo = new Repo(name, updatedDate);
                        } else {
                            // Popout from cached
                            cached.remove(id);
                            Logger.dev("LoadRepos: Update cached repo " + id);
                            repo.update(updatedDate);
                        }
                        if (repo.getId() != null) {
                            fetched.put(id, repo);
                        }
                    } catch (BaseModule.CacheModException ignored) {}

                    // Update the database
                    dbHelper.addRepoMap(fetched);
                    // The leftover cached are those removed remote, cleanup db
                    dbHelper.removeRepo(cached);
                    // Update ETag
                    prefs.edit().putString(ETAG_KEY, etag).apply();
                }

            } catch (JSONException e) {
                e.printStackTrace();
            }
        }

        magiskManager.repoMap = dbHelper.getRepoMap();

        Logger.dev("LoadRepos: Repo load done");
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        magiskManager.repoLoadDone.trigger();
    }
}
