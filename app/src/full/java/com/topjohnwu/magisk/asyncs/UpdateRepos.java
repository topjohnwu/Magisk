package com.topjohnwu.magisk.asyncs;

import android.database.Cursor;
import android.os.AsyncTask;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.container.Repo;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.net.HttpURLConnection;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.TimeZone;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class UpdateRepos {

    private static final int CPU_COUNT = Runtime.getRuntime().availableProcessors();
    private static final int CORE_POOL_SIZE = Math.max(2, CPU_COUNT - 1);
    private static final DateFormat dateFormat;

    static {
        dateFormat = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
        dateFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
    }

    private MagiskManager mm;
    private Set<String> cached;
    private ExecutorService threadPool;

    public UpdateRepos() {
        mm = Data.MM();
    }

    private void waitTasks() {
        threadPool.shutdown();
        try {
            threadPool.awaitTermination(Long.MAX_VALUE, TimeUnit.MILLISECONDS);
        } catch (InterruptedException ignored) {}
    }

    private boolean loadJSON(String jsonString) throws JSONException, ParseException {
        JSONArray jsonArray = new JSONArray(jsonString);

        // Empty page, halt
        if (jsonArray.length() == 0)
            return false;

        for (int i = 0; i < jsonArray.length(); i++) {
            JSONObject rawRepo = jsonArray.getJSONObject(i);
            String id = rawRepo.getString("name");
            Date date = dateFormat.parse(rawRepo.getString("pushed_at"));
            threadPool.execute(() -> {
                Repo repo = mm.repoDB.getRepo(id);
                try {
                    if (repo == null)
                        repo = new Repo(id);
                    else
                        cached.remove(id);
                    repo.update(date);
                    mm.repoDB.addRepo(repo);
                } catch (Repo.IllegalRepoException e) {
                    Logger.debug(e.getMessage());
                    mm.repoDB.removeRepo(id);
                }
            });
        }
        return true;
    }

    /* We sort repos by last push, which means that we only need to check whether the
     * first page is updated to determine whether the online repo database is changed
     */
    private boolean loadPage(int page) {
        Map<String, String> header = new HashMap<>();
        if (page == 0)
            header.put(Const.Key.IF_NONE_MATCH, mm.prefs.getString(Const.Key.ETAG_KEY, ""));
        String url = Utils.fmt(Const.Url.REPO_URL, page + 1);

        try {
            HttpURLConnection conn = WebService.request(url, header);
            // No updates
            if (conn.getResponseCode() == HttpURLConnection.HTTP_NOT_MODIFIED)
                return false;
            // Current page is the last page
            if (!loadJSON(WebService.getString(conn)))
                return true;
        } catch (Exception e) {
            // Should not happen, but if exception occurs, page load fails
            return false;
        }

        // Update ETAG
        if (page == 0) {
            String etag = header.get(Const.Key.ETAG_KEY);
            etag = etag.substring(etag.indexOf('\"'), etag.lastIndexOf('\"') + 1);
            mm.prefs.edit().putString(Const.Key.ETAG_KEY, etag).apply();
        }

        String links = header.get(Const.Key.LINK_KEY);
        return links == null || !links.contains("next") || loadPage(page + 1);
    }

    private void fullReload() {
        Cursor c = mm.repoDB.getRawCursor();
        while (c.moveToNext()) {
            Repo repo = new Repo(c);
            threadPool.execute(() -> {
                try {
                    repo.update();
                    mm.repoDB.addRepo(repo);
                } catch (Repo.IllegalRepoException e) {
                    Logger.debug(e.getMessage());
                    mm.repoDB.removeRepo(repo);
                }
            });
        }
        waitTasks();
    }

    public void exec(boolean force) {
        Topic.reset(Topic.REPO_LOAD_DONE);
        AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
            cached = Collections.synchronizedSet(mm.repoDB.getRepoIDSet());
            threadPool = Executors.newFixedThreadPool(CORE_POOL_SIZE);

            if (loadPage(0)) {
                waitTasks();
                // The leftover cached means they are removed from online repo
                mm.repoDB.removeRepo(cached);
            } else if (force) {
                fullReload();
            }
            Topic.publish(Topic.REPO_LOAD_DONE);
        });
    }

    public void exec() {
        exec(false);
    }
}
