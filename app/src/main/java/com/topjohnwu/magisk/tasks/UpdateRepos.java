package com.topjohnwu.magisk.tasks;

import android.database.Cursor;
import android.util.Pair;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.data.database.RepoDatabaseHelper;
import com.topjohnwu.magisk.model.entity.Repo;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.net.Networking;
import com.topjohnwu.net.Request;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.net.HttpURLConnection;
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

import androidx.annotation.NonNull;
import io.reactivex.Single;

@Deprecated
public class UpdateRepos {

    @NonNull
    private final RepoDatabaseHelper repoDB;
    private Set<String> cached;
    private Queue<Pair<String, Date>> moduleQueue;

    public UpdateRepos(@NonNull RepoDatabaseHelper repoDatabase) {
        repoDB = repoDatabase;
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
                } catch (ExecutionException ignored) {
                }
                break;
            }
        }
    }

    /**
     * Static instance of (Simple)DateFormat is not threadsafe so in order to make it safe it needs
     * to be created beforehand on the same thread where it'll be used.
     * See https://stackoverflow.com/a/18383395
     */
    private static SimpleDateFormat getDateFormat() {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
        format.setTimeZone(TimeZone.getTimeZone("UTC"));
        return format;
    }

    /* We sort repos by last push, which means that we only need to check whether the
     * first page is updated to determine whether the online repo database is changed
     */
    private boolean parsePage(int page) {
        Request req = Networking.get(Utils.INSTANCE.fmt(Const.Url.REPO_URL, page + 1));
        if (page == 0) {
            String etag = Config.getEtagKey();
            if (!etag.isEmpty())
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
            SimpleDateFormat dateFormat = getDateFormat();

            for (int i = 0; i < res.getResult().length(); i++) {
                JSONObject rawRepo = res.getResult().getJSONObject(i);
                String id = rawRepo.getString("name");
                Date date = dateFormat.parse(rawRepo.getString("pushed_at"));
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
                Config.setEtagKey(etag);
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
                Repo repo = repoDB.getRepo(pair.first);
                try {
                    if (repo == null)
                        repo = new Repo(pair.first);
                    else
                        cached.remove(pair.first);
                    repo.update(pair.second);
                    repoDB.addRepo(repo);
                } catch (Repo.IllegalRepoException e) {
                    Logger.debug(e.getMessage());
                    repoDB.removeRepo(pair.first);
                }
            }
        });
        return true;
    }

    private void fullReload() {
        Cursor c = repoDB.getRawCursor();
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
                    repoDB.addRepo(repo);
                } catch (Repo.IllegalRepoException e) {
                    Logger.debug(e.getMessage());
                    repoDB.removeRepo(repo);
                }
            }
        });
    }

    public Single<Boolean> exec(boolean force) {
        return Single.fromCallable(() -> {
            cached = Collections.synchronizedSet(repoDB.getRepoIDSet());
            moduleQueue = new ConcurrentLinkedQueue<>();

            if (loadPages()) {
                // The leftover cached means they are removed from online repo
                repoDB.removeRepo(cached);
            } else if (force) {
                fullReload();
            }
            return force; // not important
        });
    }

    public Single<Boolean> exec() {
        return exec(false);
    }
}
