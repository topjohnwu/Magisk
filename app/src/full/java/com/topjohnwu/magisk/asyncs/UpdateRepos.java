package com.topjohnwu.magisk.asyncs;

import android.database.Cursor;
import android.os.AsyncTask;
import android.text.TextUtils;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.ReposFragment;
import com.topjohnwu.magisk.container.Repo;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.net.HttpURLConnection;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;

public class UpdateRepos extends ParallelTask<Void, Void, Void> {

    private static final int CHECK_ETAG = 0;
    private static final int LOAD_NEXT = 1;
    private static final int LOAD_PREV = 2;
    private static final DateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);

    private MagiskManager mm;
    private List<String> etags, newEtags = new LinkedList<>();
    private Set<String> cached;
    private boolean forceUpdate;
    private AtomicInteger taskCount = new AtomicInteger(0);
    final private Object allDone = new Object();

    public UpdateRepos(boolean force) {
        mm = MagiskManager.get();
        mm.repoLoadDone.reset();
        forceUpdate = force;
    }

    private void queueTask(Runnable task) {
        // Thread pool's queue has an upper bound, batch it with 64 tasks
        while (taskCount.get() >= 64) {
            waitTasks();
        }
        taskCount.incrementAndGet();
        AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
            task.run();
            if (taskCount.decrementAndGet() == 0) {
                synchronized (allDone) {
                    allDone.notify();
                }
            }
        });
    }

    private void waitTasks() {
        if (taskCount.get() == 0)
            return;
        synchronized (allDone) {
            try {
                allDone.wait();
            } catch (InterruptedException e) {
                // Wait again
                waitTasks();
            }
        }
    }

    private boolean loadJSON(String jsonString) throws JSONException, ParseException {
        JSONArray jsonArray = new JSONArray(jsonString);

        // Empty page, halt
        if (jsonArray.length() == 0)
            return false;

        for (int i = 0; i < jsonArray.length(); i++) {
            JSONObject rawRepo = jsonArray.getJSONObject(i);
            String id = rawRepo.getString("description");
            String name = rawRepo.getString("name");
            Date date = dateFormat.parse(rawRepo.getString("pushed_at"));
            Set<String> set = Collections.synchronizedSet(cached);
            queueTask(() -> {
                Repo repo = mm.repoDB.getRepo(id);
                try {
                    if (repo == null)
                        repo = new Repo(name);
                    else
                        set.remove(id);
                    repo.update(date);
                    mm.repoDB.addRepo(repo);
                    publishProgress();
                } catch (Repo.IllegalRepoException e) {
                    Logger.debug(e.getMessage());
                    mm.repoDB.removeRepo(id);
                }
            });
        }
        return true;
    }

    private boolean loadPage(int page, int mode) {
        Map<String, String> header = new HashMap<>();
        if (mode == CHECK_ETAG && page < etags.size())
            header.put(Const.Key.IF_NONE_MATCH, etags.get(page));
        String url = Utils.fmt(Const.Url.REPO_URL, page + 1);

        try {
            HttpURLConnection conn = WebService.request(url, header);
            if (conn.getResponseCode() == HttpURLConnection.HTTP_NOT_MODIFIED) {
                // Current page is not updated, check the next page
                return loadPage(page + 1, CHECK_ETAG);
            }
            if (!loadJSON(WebService.getString(conn)))
                return mode != CHECK_ETAG;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }

        /* If one page is updated, we force update all pages */

        // Update ETAG
        String etag = header.get(Const.Key.ETAG_KEY);
        etag = etag.substring(etag.indexOf('\"'), etag.lastIndexOf('\"') + 1);
        if (mode == LOAD_PREV) {
            // We are loading a previous page, push the new tag to the front
            newEtags.add(0, etag);
        } else {
            newEtags.add(etag);
        }

        String links = header.get(Const.Key.LINK_KEY);
        if (links != null) {
            for (String s : links.split(", ")) {
                if (mode != LOAD_PREV && s.contains("next")) {
                    // Force load all next pages
                    loadPage(page + 1, LOAD_NEXT);
                }
                if (mode != LOAD_NEXT && s.contains("prev")) {
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
    protected void onPreExecute() {
        mm.repoLoadDone.setPending();
    }

    @Override
    protected Void doInBackground(Void... voids) {
        etags = Arrays.asList(mm.prefs.getString(Const.Key.ETAG_KEY, "").split(","));
        cached = mm.repoDB.getRepoIDSet();

        if (loadPage(0, CHECK_ETAG)) {
            waitTasks();

            // The leftover cached means they are removed from online repo
            mm.repoDB.removeRepo(cached);

            // Update ETag
            mm.prefs.edit().putString(Const.Key.ETAG_KEY, TextUtils.join(",", newEtags)).apply();
        } else if (forceUpdate) {
            Cursor c = mm.repoDB.getRawCursor();
            while (c.moveToNext()) {
                Repo repo = new Repo(c);
                queueTask(() -> {
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
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        mm.repoLoadDone.publish();
        super.onPostExecute(v);
    }
}
