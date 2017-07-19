package com.topjohnwu.magisk.asyncs;

import android.content.Context;
import android.content.SharedPreferences;
import android.text.TextUtils;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.module.BaseModule;
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.ValueSortedMap;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

public class LoadRepos extends ParallelTask<Void, Void, Void> {

    public static final String ETAG_KEY = "ETag";

    private static final String REPO_URL = "https://api.github.com/users/Magisk-Modules-Repo/repos?per_page=100&page=%d";
    private static final String IF_NONE_MATCH = "If-None-Match";
    private static final String LINK_KEY = "Link";

    private static final int CHECK_ETAG = 0;
    private static final int LOAD_NEXT = 1;
    private static final int LOAD_PREV = 2;

    private List<String> etags;
    private ValueSortedMap<String, Repo> cached, fetched;
    private RepoDatabaseHelper repoDB;
    private SharedPreferences prefs;

    public LoadRepos(Context context) {
        super(context);
        prefs = getMagiskManager().prefs;
        String prefsPath = context.getApplicationInfo().dataDir + "/shared_prefs";
        repoDB = new RepoDatabaseHelper(context);
        // Legacy data cleanup
        File old = new File(prefsPath, "RepoMap.xml");
        if (old.exists() || !prefs.getString("repomap", "empty").equals("empty")) {
            old.delete();
            prefs.edit().remove("version").remove("repomap").remove(ETAG_KEY).apply();
            repoDB.clearRepo();
        }
        etags = new ArrayList<>(
                Arrays.asList(prefs.getString(ETAG_KEY, "").split(",")));
    }

    private void loadJSON(String jsonString) throws Exception {
        JSONArray jsonArray = new JSONArray(jsonString);

        for (int i = 0; i < jsonArray.length(); i++) {
            JSONObject jsonobject = jsonArray.getJSONObject(i);
            String id = jsonobject.getString("description");
            String name = jsonobject.getString("name");
            String lastUpdate = jsonobject.getString("pushed_at");
            SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
            Date updatedDate = format.parse(lastUpdate);
            Repo repo = cached.get(id);
            try {
                if (repo == null) {
                    Logger.dev("LoadRepos: Create new repo " + id);
                    repo = new Repo(name, updatedDate);
                } else {
                    // Popout from cached
                    cached.remove(id);
                    repo.update(updatedDate);
                }
                if (repo.getId() != null) {
                    fetched.put(id, repo);
                }
            } catch (BaseModule.CacheModException ignored) {}
        }
    }

    private boolean loadPage(int page, String url, int mode) {
        Logger.dev("LoadRepos: Loading page: " + (page + 1));
        Map<String, String> header = new HashMap<>();
        if (mode == CHECK_ETAG && page < etags.size() && !TextUtils.isEmpty(etags.get(page))) {
            Logger.dev("ETAG: " + etags.get(page));
            header.put(IF_NONE_MATCH, etags.get(page));
        }
        if (url == null) {
            url = String.format(Locale.US, REPO_URL, page + 1);
        }
        String jsonString = WebService.request(url, WebService.GET, header, true);
        if (TextUtils.isEmpty(jsonString)) {
            // At least check the pages we know
            return page + 1 < etags.size() && loadPage(page + 1, null, CHECK_ETAG);
        }

        // The request succeed, parse the new stuffs
        try {
            loadJSON(jsonString);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }

        // Update the ETAG
        String newEtag = header.get(ETAG_KEY);
        newEtag = newEtag.substring(newEtag.indexOf('\"'), newEtag.lastIndexOf('\"') + 1);
        Logger.dev("New ETAG: " + newEtag);
        if (page < etags.size()) {
            etags.set(page, newEtag);
        } else {
            etags.add(newEtag);
        }

        String links = header.get(LINK_KEY);
        if (links != null) {
            if (mode == CHECK_ETAG || mode == LOAD_NEXT) {
                // Try to check next page URL
                url = null;
                for (String s : links.split(", ")) {
                    if (s.contains("next")) {
                        url = s.substring(s.indexOf("<") + 1, s.indexOf(">; "));
                        break;
                    }
                }
                if (url != null) {
                    loadPage(page + 1, url, LOAD_NEXT);
                }
            }

            if (mode == CHECK_ETAG || mode == LOAD_PREV) {
                // Try to check prev page URL
                url = null;
                for (String s : links.split(", ")) {
                    if (s.contains("prev")) {
                        url = s.substring(s.indexOf("<") + 1, s.indexOf(">; "));
                        break;
                    }
                }
                if (url != null) {
                    loadPage(page - 1, url, LOAD_PREV);
                }
            }
        }

        return true;
    }

    @Override
    protected Void doInBackground(Void... voids) {
        MagiskManager magiskManager = getMagiskManager();
        if (magiskManager == null) return null;
        Logger.dev("LoadRepos: Loading repos");

        cached = repoDB.getRepoMap(false);
        fetched = new ValueSortedMap<>();

        if (!loadPage(0, null, CHECK_ETAG)) {
            magiskManager.repoMap = repoDB.getRepoMap();
            Logger.dev("LoadRepos: No updates, use DB");
            return null;
        }

        repoDB.addRepoMap(fetched);
        repoDB.removeRepo(cached);

        // Update ETag
        StringBuilder etagBuilder = new StringBuilder();
        for (int i = 0; i < etags.size(); ++i) {
            if (i != 0) etagBuilder.append(",");
            etagBuilder.append(etags.get(i));
        }
        prefs.edit().putString(ETAG_KEY, etagBuilder.toString()).apply();

        magiskManager.repoMap = repoDB.getRepoMap();
        Logger.dev("LoadRepos: Done");
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        MagiskManager magiskManager = getMagiskManager();
        if (magiskManager == null) return;
        magiskManager.repoLoadDone.trigger();
        super.onPostExecute(v);
    }
}
