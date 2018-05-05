package com.topjohnwu.magisk.asyncs;

import android.os.AsyncTask;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.ReposFragment;
import com.topjohnwu.magisk.container.Repo;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.File;
import java.net.HttpURLConnection;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

public class UpdateRepos extends ParallelTask<Void, Void, Void> {

    private static final int CHECK_ETAG = 0;
    private static final int LOAD_NEXT = 1;
    private static final int LOAD_PREV = 2;
    private static final DateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);

    private MagiskManager mm;
    private List<String> cached, etags, newEtags = new LinkedList<>();
    private boolean forceUpdate;
    private AtomicInteger taskCount = new AtomicInteger(0);
    final private Object allDone = new Object();

    public UpdateRepos(boolean force) {
        mm = MagiskManager.get();
        mm.repoLoadDone.reset();
        // Legacy data cleanup
        File old = new File(mm.getApplicationInfo().dataDir + "/shared_prefs", "RepoMap.xml");
        if (old.exists() || mm.prefs.getString("repomap", null) != null) {
            old.delete();
            mm.prefs.edit().remove("version").remove("repomap").remove(Const.Key.ETAG_KEY).apply();
            mm.repoDB.clearRepo();
        }
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

    private void loadJSON(String jsonString) throws Exception {
        JSONArray jsonArray = new JSONArray(jsonString);

        // Empty page, throw error
        if (jsonArray.length() == 0)
            throw new Exception();

        for (int i = 0; i < jsonArray.length(); i++) {
            JSONObject rawRepo = jsonArray.getJSONObject(i);
            String id = rawRepo.getString("description");
            String name = rawRepo.getString("name");
            Date date = dateFormat.parse(rawRepo.getString("pushed_at"));
            final List<String> c = cached;
            queueTask(() -> {
                Repo repo = mm.repoDB.getRepo(id);
                Boolean updated;
                try {
                    if (repo == null) {
                        repo = new Repo(name, date);
                        updated = true;
                    } else {
                        // Popout from cached
                        synchronized (c) {
                            c.remove(id);
                        }
                        if (forceUpdate) {
                            repo.update();
                            updated = true;
                        } else {
                            updated = repo.update(date);
                        }
                    }
                    if (updated) {
                        mm.repoDB.addRepo(repo);
                        publishProgress();
                    }
                    if (!id.equals(repo.getId())) {
                        Logger.error("Repo [" + name + "] rid=[" + id + "] id=[" + repo.getId() + "] mismatch");
                    }
                } catch (Repo.IllegalRepoException e) {
                    Logger.error(e.getMessage());
                    mm.repoDB.removeRepo(id);
                }
            });
        }
    }

    private boolean loadPage(int page, int mode) {
        Map<String, String> header = new HashMap<>();
        String etag = (mode == CHECK_ETAG && page < etags.size()) ? etags.get(page) : "";
        header.put(Const.Key.IF_NONE_MATCH, etag);
        String url = Utils.fmt(Const.Url.REPO_URL, page + 1);
        HttpURLConnection conn = WebService.request(url, header);

        try {
            if (conn == null)
                throw new Exception();
            if (conn.getResponseCode() == HttpURLConnection.HTTP_NOT_MODIFIED) {
                // Current page is not updated, check the next page
                return page + 1 < etags.size() && loadPage(page + 1, CHECK_ETAG);
            }
            loadJSON(WebService.getString(conn));
        } catch (Exception e) {
            e.printStackTrace();
            // Don't continue
            return true;
        }

        /* If one page is updated, we force update all pages */

        // Update ETAG
        etag = header.get(Const.Key.ETAG_KEY);
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
        etags = new ArrayList<>(Arrays.asList(mm.prefs.getString(Const.Key.ETAG_KEY, "").split(",")));
        cached = mm.repoDB.getRepoIDList();

        if (!loadPage(0, CHECK_ETAG)) {
            // Nothing changed online
            if (forceUpdate) {
                for (String id : cached) {
                    if (id == null) continue;
                    queueTask(() -> {
                        Repo repo = mm.repoDB.getRepo(id);
                        try {
                            repo.update();
                            mm.repoDB.addRepo(repo);
                        } catch (Repo.IllegalRepoException e) {
                            Logger.error(e.getMessage());
                            mm.repoDB.removeRepo(repo);
                        }
                    });
                }
            }
            waitTasks();
        } else {
            waitTasks();

            // The leftover cached means they are removed from online repo
            mm.repoDB.removeRepo(cached);

            // Update ETag
            StringBuilder etagBuilder = new StringBuilder();
            for (int i = 0; i < newEtags.size(); ++i) {
                if (i != 0) etagBuilder.append(",");
                etagBuilder.append(newEtags.get(i));
            }
            mm.prefs.edit().putString(Const.Key.ETAG_KEY, etagBuilder.toString()).apply();
        }
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        mm.repoLoadDone.publish();
        super.onPostExecute(v);
    }
}
