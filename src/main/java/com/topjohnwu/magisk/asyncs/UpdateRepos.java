package com.topjohnwu.magisk.asyncs;

import android.content.SharedPreferences;
import android.os.AsyncTask;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.ReposFragment;
import com.topjohnwu.magisk.container.Repo;
import com.topjohnwu.magisk.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Logger;
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

    private static final int CHECK_ETAG = 0;
    private static final int LOAD_NEXT = 1;
    private static final int LOAD_PREV = 2;

    private List<String> cached, etags, newEtags = new ArrayList<>();
    private RepoDatabaseHelper repoDB;
    private SharedPreferences prefs;
    private boolean forceUpdate;

    private int tasks = 0;

    public UpdateRepos(boolean force) {
        MagiskManager mm = MagiskManager.get();
        prefs = mm.prefs;
        repoDB = mm.repoDB;
        mm.repoLoadDone.hasPublished = false;
        // Legacy data cleanup
        File old = new File(mm.getApplicationInfo().dataDir + "/shared_prefs", "RepoMap.xml");
        if (old.exists() || prefs.getString("repomap", null) != null) {
            old.delete();
            prefs.edit().remove("version").remove("repomap").remove(Const.Key.ETAG_KEY).apply();
            repoDB.clearRepo();
        }
        forceUpdate = force;
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
            ++tasks;
            AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
                Repo repo = repoDB.getRepo(id);
                Boolean updated;
                try {
                    if (repo == null) {
                        repo = new Repo(name, updatedDate);
                        updated = true;
                    } else {
                        // Popout from cached
                        cached.remove(id);
                        if (forceUpdate) {
                            repo.update();
                            updated = true;
                        } else {
                            updated = repo.update(updatedDate);
                        }
                    }
                    if (updated) {
                        repoDB.addRepo(repo);
                        publishProgress();
                    }
                    if (!id.equals(repo.getId())) {
                        Logger.error("Repo [" + name + "] id=[" + repo.getId() + "] has illegal repo id");
                    }
                } catch (Repo.IllegalRepoException e) {
                    Logger.error(e.getMessage());
                    repoDB.removeRepo(id);
                }
                --tasks;
            });
        }
    }

    private boolean loadPage(int page, int mode) {
        Map<String, String> header = new HashMap<>();
        String etag = "";
        if (mode == CHECK_ETAG && page < etags.size()) {
            etag = etags.get(page);
        }
        header.put(Const.Key.IF_NONE_MATCH, etag);
        String url = String.format(Locale.US, Const.Url.REPO_URL, page + 1);
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
        etag = header.get(Const.Key.ETAG_KEY);
        etag = etag.substring(etag.indexOf('\"'), etag.lastIndexOf('\"') + 1);
        newEtags.add(etag);

        String links = header.get(Const.Key.LINK_KEY);
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

    private Void waitTasks() {
        while (tasks > 0) {
            try {
                Thread.sleep(5);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        return null;
    }

    @Override
    protected void onProgressUpdate(Void... values) {
        if (ReposFragment.adapter != null)
            ReposFragment.adapter.notifyDBChanged();
    }

    @Override
    protected Void doInBackground(Void... voids) {
        etags = new ArrayList<>(Arrays.asList(prefs.getString(Const.Key.ETAG_KEY, "").split(",")));
        cached = repoDB.getRepoIDList();

        if (!loadPage(0, CHECK_ETAG)) {
            // Nothing changed online
            if (forceUpdate) {
                for (String id : cached) {
                    if (id == null) continue;
                    ++tasks;
                    AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
                        Repo repo = repoDB.getRepo(id);
                        try {
                            repo.update();
                            repoDB.addRepo(repo);
                        } catch (Repo.IllegalRepoException e) {
                            Logger.error(e.getMessage());
                            repoDB.removeRepo(repo);
                        }
                        --tasks;
                    });
                }
            }
            return waitTasks();
        }

        // Wait till all tasks are done
        waitTasks();

        // The leftover cached means they are removed from online repo
        repoDB.removeRepo(cached);

        // Update ETag
        StringBuilder etagBuilder = new StringBuilder();
        for (int i = 0; i < newEtags.size(); ++i) {
            if (i != 0) etagBuilder.append(",");
            etagBuilder.append(newEtags.get(i));
        }
        prefs.edit().putString(Const.Key.ETAG_KEY, etagBuilder.toString()).apply();
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        MagiskManager.get().repoLoadDone.publish();
        super.onPostExecute(v);
    }
}
