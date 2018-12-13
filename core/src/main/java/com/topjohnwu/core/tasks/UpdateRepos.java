package com.topjohnwu.core.tasks;

import android.database.Cursor;
import android.os.AsyncTask;

import com.topjohnwu.core.App;
import com.topjohnwu.core.Const;
import com.topjohnwu.core.container.Repo;
import com.topjohnwu.core.utils.Logger;
import com.topjohnwu.core.utils.Topic;
import com.topjohnwu.core.utils.Utils;
import com.topjohnwu.net.Networking;
import com.topjohnwu.net.Request;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.net.HttpURLConnection;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Collections;
import java.util.Date;
import java.util.Locale;
import java.util.Set;
import java.util.TimeZone;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class UpdateRepos {
    private static final int CPU_COUNT = Runtime.getRuntime().availableProcessors();
    private static final int CORE_POOL_SIZE = Math.max(2, CPU_COUNT - 1);
    private static final DateFormat dateFormat;

    private App app = App.self;
    private Set<String> cached;
    private ExecutorService threadPool;

    static {
        dateFormat = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
        dateFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
    }

    private void waitTasks() {
        threadPool.shutdown();
        while (true) {
            try {
                if (threadPool.awaitTermination(Long.MAX_VALUE, TimeUnit.NANOSECONDS))
                    break;
            } catch (InterruptedException ignored) {}
        }
    }

    private void loadJSON(JSONArray array) throws JSONException, ParseException {
        for (int i = 0; i < array.length(); i++) {
            JSONObject rawRepo = array.getJSONObject(i);
            String id = rawRepo.getString("name");
            Date date = dateFormat.parse(rawRepo.getString("pushed_at"));
            threadPool.execute(() -> {
                Repo repo = app.repoDB.getRepo(id);
                try {
                    if (repo == null)
                        repo = new Repo(id);
                    else
                        cached.remove(id);
                    repo.update(date);
                    app.repoDB.addRepo(repo);
                } catch (Repo.IllegalRepoException e) {
                    Logger.debug(e.getMessage());
                    app.repoDB.removeRepo(id);
                }
            });
        }
    }

    /* We sort repos by last push, which means that we only need to check whether the
     * first page is updated to determine whether the online repo database is changed
     */
    private boolean loadPage(int page) {
        Request req = Networking.get(Utils.fmt(Const.Url.REPO_URL, page + 1));
        if (page == 0) {
            String etag = app.prefs.getString(Const.Key.ETAG_KEY, null);
            if (etag != null)
                req.addHeaders(Const.Key.IF_NONE_MATCH, etag);
        }
        Request.Result<JSONArray> res = req.execForJSONArray();
        // JSON not updated
        if (res.getCode() == HttpURLConnection.HTTP_NOT_MODIFIED)
            return false;
        // Network error
        if (res.getResult() == null) {
            cached.clear();
            return true;
        }
        // Current page is the last page
        if (res.getResult().length() == 0)
            return true;

        try {
            loadJSON(res.getResult());
        } catch (JSONException | ParseException e) {
            // Should not happen, but if exception occurs, page load fails
            return false;
        }

        // Update ETAG
        if (page == 0) {
            String etag = res.getConnection().getHeaderField(Const.Key.ETAG_KEY);
            if (etag != null) {
                etag = etag.substring(etag.indexOf('\"'), etag.lastIndexOf('\"') + 1);
                app.prefs.edit().putString(Const.Key.ETAG_KEY, etag).apply();
            }
        }

        String links = res.getConnection().getHeaderField(Const.Key.LINK_KEY);
        return links == null || !links.contains("next") || loadPage(page + 1);
    }

    private boolean loadPages() {
        return loadPage(0);
    }

    private void fullReload() {
        Cursor c = app.repoDB.getRawCursor();
        while (c.moveToNext()) {
            Repo repo = new Repo(c);
            threadPool.execute(() -> {
                try {
                    repo.update();
                    app.repoDB.addRepo(repo);
                } catch (Repo.IllegalRepoException e) {
                    Logger.debug(e.getMessage());
                    app.repoDB.removeRepo(repo);
                }
            });
        }
        waitTasks();
    }

    public void exec(boolean force) {
        Topic.reset(Topic.REPO_LOAD_DONE);
        AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
            cached = Collections.synchronizedSet(app.repoDB.getRepoIDSet());
            threadPool = Executors.newFixedThreadPool(CORE_POOL_SIZE);

            if (loadPages()) {
                waitTasks();
                // The leftover cached means they are removed from online repo
                app.repoDB.removeRepo(cached);
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
