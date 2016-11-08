package com.topjohnwu.magisk.utils;

import android.content.Context;
import android.content.SharedPreferences;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.module.BaseModule;
import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.module.Repo;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Comparator;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.TreeMap;

public class ModuleHelper {
    public static final String MAGISK_PATH = "/magisk";

    private static final String file_key = "RepoMap";
    private static final String key = "repomap";
    private static TreeMap<String, Repo> repoMap = new TreeMap<>(new ModuleComparator());
    private static TreeMap<String, Module> moduleMap = new TreeMap<>(new ModuleComparator());


    public static void createModuleMap() {
        Logger.dev("ModuleHelper: Loading modules");

        moduleMap.clear();

        for (String path : Utils.getModList(MAGISK_PATH)) {
            Logger.dev("ModuleHelper: Adding modules from " + path);
            Module module;
            try {
                module = new Module(path);
                moduleMap.put(module.getId(), module);
            } catch (BaseModule.CacheModException ignored) {}
        }

        Logger.dev("ModuleHelper: Module load done");
    }

    public static void createRepoMap(Context context) {
        Logger.dev("ModuleHelper: Loading repos");

        repoMap.clear();

        Gson gson = new Gson();
        SharedPreferences prefs = context.getSharedPreferences(file_key, Context.MODE_PRIVATE);
        String jsonString = prefs.getString(key, null);

        TreeMap<String, Repo> cached = null;

        if (jsonString != null) {
            cached = gson.fromJson(jsonString, new TypeToken< TreeMap<String, Repo> >(){}.getType());
        }

        if (cached == null) {
            cached = new TreeMap<>(new ModuleComparator());
        }

        // Making a request to url and getting response
        String jsonStr = WebRequest.makeWebServiceCall(context.getString(R.string.url_main) + Utils.getToken(), WebRequest.GET);
        if (jsonStr != null && !jsonStr.isEmpty()) {
            try {
                JSONArray jsonArray = new JSONArray(jsonStr);
                for (int i = 0; i < jsonArray.length(); i++) {
                    JSONObject jsonobject = jsonArray.getJSONObject(i);
                    String id = jsonobject.getString("description");
                    String name = jsonobject.getString("name");
                    String lastUpdate = jsonobject.getString("pushed_at");
                    SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
                    Date updatedDate;
                    try {
                        updatedDate = format.parse(lastUpdate);
                    } catch (ParseException e) {
                        continue;
                    }
                    Repo repo = cached.get(id);
                    try {
                        if (repo == null) {
                            Logger.dev("ModuleHelper: Create new repo " + id);
                            repo = new Repo(context, name, updatedDate);
                        } else {
                            Logger.dev("ModuleHelper: Cached repo " + id);
                            repo.update(updatedDate);
                        }
                        if (repo.getId() != null) {
                            repoMap.put(id, repo);
                        }
                    } catch (BaseModule.CacheModException ignored) {}
                }
            } catch (JSONException e) {
                e.printStackTrace();
            }

            prefs.edit().putString(key, gson.toJson(repoMap)).apply();
        }

        Logger.dev("ModuleHelper: Repo load done");
    }

    public static void getModuleList(List<Module> moduleList) {
        moduleList.clear();
        moduleList.addAll(moduleMap.values());
    }

    public static void getRepoLists(List<Repo> update, List<Repo> installed, List<Repo> others) {
        update.clear();
        installed.clear();
        others.clear();
        for (Repo repo : repoMap.values()) {
            Module module = moduleMap.get(repo.getId());
            if (module != null) {
                if (repo.getVersionCode() > module.getVersionCode()) {
                    update.add(repo);
                } else {
                    installed.add(repo);
                }
            } else {
                others.add(repo);
            }
        }
    }

    public static class ModuleComparator implements Comparator<String> {
        @Override
        public int compare(String o1, String o2) {
            return o1.toLowerCase().compareTo(o2.toLowerCase());
        }
    }
}
