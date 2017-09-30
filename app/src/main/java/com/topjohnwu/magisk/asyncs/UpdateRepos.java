package com.topjohnwu.magisk.asyncs;

import android.content.Context;
import android.content.SharedPreferences;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.ReposFragment;
import com.topjohnwu.magisk.container.BaseModule;
import com.topjohnwu.magisk.container.Repo;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.File;
import java.net.HttpURLConnection;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

public class UpdateRepos extends ParallelTask<Void, Void, Void> {

    private static final String REPO_URL = "https://api.github.com/users/Magisk-Modules-Repo/repos?per_page=100&page=%d";

    public static final String ETAG_KEY = "ETag";
    private static final String IF_NONE_MATCH = "If-None-Match";
    private static final String LINK_KEY = "Link";

    private static final int CHECK_ETAG = 0;
    private static final int LOAD_NEXT = 1;
    private static final int LOAD_PREV = 2;

    private List<String> cached, etags, newEtags = new ArrayList<>();
    private RepoDatabaseHelper repoDB;
    private SharedPreferences prefs;

    public UpdateRepos(Context context) {
        super(context);
        prefs = getMagiskManager().prefs;
        repoDB = getMagiskManager().repoDB;
        getMagiskManager().repoLoadDone.hasPublished = false;
        // Legacy data cleanup
        File old = new File(context.getApplicationInfo().dataDir + "/shared_prefs", "RepoMap.xml");
        if (old.exists() || prefs.getString("repomap", null) != null) {
            old.delete();
            prefs.edit().remove("version").remove("repomap").remove(ETAG_KEY).apply();
            repoDB.clearRepo();
        }
    }

    private void loadJSON(String jsonString) throws Exception {
        JSONArray jsonArray = new JSONArray(jsonString);

        // Empty page, throw error
        if (jsonArray.length() == 0) throw new Exception();

        for (int i = 0; i < jsonArray.length(); i++) {
            JSONObject jsonobject = jsonArray.getJSONObject(i);
            String id = jsonobject.getString("description");
            String name = jsonobject.getString("name");
            String lastUpdate = jsonobject.getString("pushed_at");
            SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
            Date updatedDate = format.parse(lastUpdate);
            Repo repo = repoDB.getRepo(id);
            Boolean updated;
            if (repo == null) {
                repo = new Repo(name, updatedDate);
                updated = true;
            } else {
                // Popout from cached
                cached.remove(id);
                updated = repo.update(updatedDate);
            }
            if (updated) {
                repoDB.addRepo(repo);
                publishProgress();
            }
        }
    }

    private boolean loadPage(int page, int mode) {
        Map<String, String> header = new HashMap<>();
        String etag = "";
        if (mode == CHECK_ETAG && page < etags.size()) {
            etag = etags.get(page);
        }
        header.put(IF_NONE_MATCH, etag);
        String url = String.format(Locale.US, REPO_URL, page + 1);
        HttpURLConnection conn = WebService.request(url, header);

        try {
            if (conn == null) throw new Exception();
            if (conn.getResponseCode() == HttpURLConnection.HTTP_NOT_MODIFIED) {
                newEtags.add(etag);
                return page + 1 < etags.size() && loadPage(page + 1, CHECK_ETAG);
            }
            loadJSON(WebService.getString(conn));
        } catch (Exception e) {
            e.printStackTrace();
            // Don't continue
            return true;
        }

        // Update ETAG
        etag = header.get(ETAG_KEY);
        etag = etag.substring(etag.indexOf('\"'), etag.lastIndexOf('\"') + 1);
        newEtags.add(etag);

        String links = header.get(LINK_KEY);
        if (links != null) {
            for (String s : links.split(", ")) {
                if (mode != LOAD_PREV && s.contains("next")) {
                    // Force load all next pages
                    loadPage(page + 1, LOAD_NEXT);
                } else if (mode != LOAD_NEXT && s.contains("prev")) {
                    // Back propagation
                    loadPage(page - 1, LOAD_PREV);
                }
            }
        }
        return true;
    }

    @Override
    protected void onProgressUpdate(Void... values) {
        if (ReposFragment.adapter != null)
            ReposFragment.adapter.notifyDBChanged();
    }

    @Override
    protected Void doInBackground(Void... voids) {
        etags = new ArrayList<>(Arrays.asList(prefs.getString(ETAG_KEY, "").split(",")));
        cached = repoDB.getRepoIDList();

        if (!loadPage(0, CHECK_ETAG)) {
            // Nothing changed
            return null;
        }

        // The leftover cached means they are removed from online repo
        repoDB.removeRepo(cached);

        // Update ETag
        StringBuilder etagBuilder = new StringBuilder();
        for (int i = 0; i < newEtags.size(); ++i) {
            if (i != 0) etagBuilder.append(",");
            etagBuilder.append(newEtags.get(i));
        }
        prefs.edit().putString(ETAG_KEY, etagBuilder.toString()).apply();
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
