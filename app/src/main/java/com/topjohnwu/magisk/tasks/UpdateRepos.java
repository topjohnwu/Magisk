package com.topjohnwu.magisk.tasks;

import android.database.Cursor;
import android.util.Pair;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.container.Repo;
import com.topjohnwu.magisk.utils.Event;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;
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
import java.util.Queue;
import java.util.Set;
import java.util.TimeZone;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;

public class UpdateRepos {
    private static final DateFormat DATE_FORMAT;

    private App app = App.self;
    private Set<String> cached;
    private Queue<Pair<String, Date>> moduleQueue;

    static {
        DATE_FORMAT = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
        DATE_FORMAT.setTimeZone(TimeZone.getTimeZone("UTC"));
    }

    private void runTasks(Runnable task) {
        Future[] futures = new Future[App.THREAD_POOL.getMaximumPoolSize() - 1];
        for (int i = 0; i < futures.length; ++i) {
            futures[i] = App.THREAD_POOL.submit(task);
        }
        for (Future f : futures) {
            while (true) {
                try {
                    f.get();
                } catch (InterruptedException e) {
                    continue;
                } catch (ExecutionException ignored) {}
                break;
            }
        }
    }

    /* We sort repos by last push, which means that we only need to check whether the
     * first page is updated to determine whether the online repo database is changed
     */
    private boolean parsePage(int page) {
        Request req = Networking.get(Utils.fmt(Const.Url.REPO_URL, page + 1));
        if (page == 0) {
            String etag = Config.get(Config.Key.ETAG_KEY);
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
            for (int i = 0; i < res.getResult().length(); i++) {
                JSONObject rawRepo = res.getResult().getJSONObject(i);
                String id = rawRepo.getString("name");
                Date date = DATE_FORMAT.parse(rawRepo.getString("pushed_at"));
                moduleQueue.offer(new Pair<>(id, date));
            }
        } catch (JSONException | ParseException e) {
            // Should not happen, but if exception occurs, page load fails
            return false;
        }

        // Update ETAG
        if (page == 0) {
            String etag = res.getConnection().getHeaderField(Config.Key.ETAG_KEY);
            if (etag != null) {
                etag = etag.substring(etag.indexOf('\"'), etag.lastIndexOf('\"') + 1);
                Config.set(Config.Key.ETAG_KEY, etag);
            }
        }

        String links = res.getConnection().getHeaderField(Const.Key.LINK_KEY);
        return links == null || !links.contains("next") || parsePage(page + 1);
    }

    private boolean loadPages() {
        if (!parsePage(0))
            return false;
        runTasks(() -> {
            while (true) {
                Pair<String, Date> pair = moduleQueue.poll();
                if (pair == null)
                    return;
                Repo repo = app.repoDB.getRepo(pair.first);
                try {
                    if (repo == null)
                        repo = new Repo(pair.first);
                    else
                        cached.remove(pair.first);
                    repo.update(pair.second);
                    app.repoDB.addRepo(repo);
                } catch (Repo.IllegalRepoException e) {
                    Logger.debug(e.getMessage());
                    app.repoDB.removeRepo(pair.first);
                }
            }
        });
        return true;
    }

    private void fullReload() {
        Cursor c = app.repoDB.getRawCursor();
        runTasks(() -> {
            while (true) {
                Repo repo;
                synchronized (c) {
                    if (!c.moveToNext())
                        return;
                    repo = new Repo(c);
                }
                try {
                    repo.update();
                    app.repoDB.addRepo(repo);
                } catch (Repo.IllegalRepoException e) {
                    Logger.debug(e.getMessage());
                    app.repoDB.removeRepo(repo);
                }
            }
        });
    }

    public void exec(boolean force) {
        Event.reset(Event.REPO_LOAD_DONE);
        App.THREAD_POOL.execute(() -> {
            cached = Collections.synchronizedSet(app.repoDB.getRepoIDSet());
            moduleQueue = new ConcurrentLinkedQueue<>();

            if (loadPages()) {
                // The leftover cached means they are removed from online repo
                app.repoDB.removeRepo(cached);
            } else if (force) {
                fullReload();
            }
            Event.trigger(Event.REPO_LOAD_DONE);
        });
    }

    public void exec() {
        exec(false);
    }
}
