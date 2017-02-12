package com.topjohnwu.magisk.asyncs;

import android.app.Activity;
import android.content.SharedPreferences;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.module.BaseModule;
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.ValueSortedMap;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

public class LoadRepos extends ParallelTask<Void, Void, Void> {

    public static final String ETAG_KEY = "ETag";
    public static final String VERSION_KEY = "version";
    public static final String REPO_KEY = "repomap";
    public static final String FILE_KEY = "RepoMap";
    private static final int GSON_DB_VER = 1;

    public LoadRepos(Activity context) {
        super(context);
    }

    @Override
    protected Void doInBackground(Void... voids) {
        Logger.dev("LoadRepos: Loading repos");

        SharedPreferences prefs = magiskManager.prefs;

        magiskManager.repoMap = new ValueSortedMap<>();

        Gson gson = new Gson();
        String jsonString;

        int cachedVersion = prefs.getInt(VERSION_KEY, 0);
        if (cachedVersion != GSON_DB_VER) {
            // Ignore incompatible cached database
            jsonString = null;
        } else {
            jsonString = prefs.getString(REPO_KEY, null);
        }

        Map<String, Repo> cached = null;

        if (jsonString != null) {
            cached = gson.fromJson(jsonString, new TypeToken<ValueSortedMap<String, Repo>>(){}.getType());
        }

        if (cached == null) {
            cached = new ValueSortedMap<>();
        }

        // Get cached ETag to add in the request header
        String etag = prefs.getString(ETAG_KEY, "");
        Map<String, String> header = new HashMap<>();
        header.put("If-None-Match", etag);

        // Making a request to main URL for repo info
        jsonString = WebService.request(
                magiskManager.getString(R.string.url_main), WebService.GET, null, header, false);

        if (!jsonString.isEmpty()) {
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
                            repo = new Repo(magiskManager, name, updatedDate);
                        } else {
                            Logger.dev("LoadRepos: Update cached repo " + id);
                            repo.update(updatedDate);
                        }
                        if (repo.getId() != null) {
                            magiskManager.repoMap.put(id, repo);
                        }
                    } catch (BaseModule.CacheModException ignored) {}
                }
            } catch (JSONException e) {
                e.printStackTrace();
            }
        } else {
            // Use cached if no internet or no updates
            Logger.dev("LoadRepos: No updates, use cached");
            magiskManager.repoMap.putAll(cached);
        }

        prefs.edit()
                .putInt(VERSION_KEY, GSON_DB_VER)
                .putString(REPO_KEY, gson.toJson(magiskManager.repoMap))
                .putString(ETAG_KEY, etag)
                .apply();

        Logger.dev("LoadRepos: Repo load done");
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        magiskManager.repoLoadDone.trigger();
    }
}
