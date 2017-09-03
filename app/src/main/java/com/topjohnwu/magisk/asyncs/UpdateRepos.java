package com.topjohnwu.magisk.asyncs;

import android.content.Context;
import android.content.SharedPreferences;
import android.text.TextUtils;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.module.BaseModule;
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.Logger;
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

public class UpdateRepos extends ParallelTask<Void, Void, Void> {

    public static final String ETAG_KEY = "ETag";

    private static final String REPO_URL = "https://api.github.com/users/Magisk-Modules-Repo/repos?per_page=100&page=%d";
    private static final String IF_NONE_MATCH = "If-None-Match";
    private static final String LINK_KEY = "Link";

    private static final int CHECK_ETAG = 0;
    private static final int LOAD_NEXT = 1;
    private static final int LOAD_PREV = 2;

    private List<String> etags;
    private List<String> cached;
    private RepoDatabaseHelper repoDB;
    private SharedPreferences prefs;

    public UpdateRepos(Context context) {
        super(context);
        prefs = getMagiskManager().prefs;
        repoDB = getMagiskManager().repoDB;
        String prefsPath = context.getApplicationInfo().dataDir + "/shared_prefs";
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
            Repo repo = repoDB.getRepo(id);
            try {
                Boolean updated;
                if (repo == null) {
                    Logger.dev("UpdateRepos: Create new repo " + id);
                    repo = new Repo(name, updatedDate);
                    updated = true;
                } else {
                    // Popout from cached
                    cached.remove(id);
                    updated = repo.update(updatedDate);
                }
                if (updated) {
                    repoDB.addRepo(repo);
                }
            } catch (BaseModule.CacheModException ignored) {}
        }
    }

    private boolean loadPage(int page, String url, int mode) {
        Logger.dev("UpdateRepos: Loading page: " + (page + 1));
        Map<String, String> header = new HashMap<>();
        if (mode == CHECK_ETAG && page < etags.size() && !TextUtils.isEmpty(etags.get(page))) {
            Logger.dev("ETAG: " + etags.get(page));
            header.put(IF_NONE_MATCH, etags.get(page));
        }
        if (url == null) {
            url = String.format(Locale.US, REPO_URL, page + 1);
        }
        String jsonString = WebService.getString(url, header);
        if (TextUtils.isEmpty(jsonString)) {
            // At least check the pages we know
            return page + 1 < etags.size() && loadPage(page + 1, null, CHECK_ETAG);
        }

        // The getString succeed, parse the new stuffs
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
        Logger.dev("UpdateRepos: Loading repos");

        cached = repoDB.getRepoIDList();

        if (!loadPage(0, null, CHECK_ETAG)) {
            Logger.dev("UpdateRepos: No updates, use DB");
            return null;
        }

        // The leftover cached means they are removed from online repo
        repoDB.removeRepo(cached);

        // Update ETag
        StringBuilder etagBuilder = new StringBuilder();
        for (int i = 0; i < etags.size(); ++i) {
            if (i != 0) etagBuilder.append(",");
            etagBuilder.append(etags.get(i));
        }
        prefs.edit().putString(ETAG_KEY, etagBuilder.toString()).apply();

        Logger.dev("UpdateRepos: Done");
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        MagiskManager mm = getMagiskManager();
        if (mm == null) return;
        mm.repoLoadDone.publish();
        super.onPostExecute(v);
    }
}
