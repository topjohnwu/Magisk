package com.topjohnwu.magisk.module;

import android.content.Context;
import android.content.SharedPreferences;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebRequest;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;

public class RepoHelper {
    private static List<Repo> repos = new ArrayList<>();
    private static String TAG = "Magisk";

    private static final String file_key = "RepoMap";
    private static final String key = "repomap";
    public static HashMap<String, Repo> repoMap;

    public static void createRepoMap(Context context) {
        Gson gson = new Gson();
        SharedPreferences prefs = context.getSharedPreferences(file_key, Context.MODE_PRIVATE);
        String jsonString = prefs.getString(key, null);
        if (jsonString != null) {
            repoMap = gson.fromJson(jsonString, new TypeToken< HashMap<String, Repo> >(){}.getType());
        }

        if (repoMap == null) {
            repoMap = new HashMap<>();
        }

        // Making a request to url and getting response
        String token = context.getString(R.string.some_string);
        String url1 = context.getString(R.string.url_main);
        String jsonStr = WebRequest.makeWebServiceCall(url1 + Utils.procFile(token, context), WebRequest.GET);
        if (jsonStr != null && !jsonStr.isEmpty()) {
            try {
                JSONArray jsonArray = new JSONArray(jsonStr);
                for (int i = 0; i < jsonArray.length(); i++) {
                    JSONObject jsonobject = jsonArray.getJSONObject(i);
                    String id = jsonobject.getString("description");
                    String name = jsonobject.getString("name");
                    String lastUpdate = jsonobject.getString("updated_at");
                    SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US);
                    Date updatedDate;
                    try {
                        updatedDate = format.parse(lastUpdate);
                    } catch (ParseException e) {
                        continue;
                    }
                    Repo repo = repoMap.get(id);
                    if (repo == null) {
                        repoMap.put(id, new Repo(context, name, updatedDate));
                    } else {
                        repo.update(updatedDate);
                    }
                }
            } catch (JSONException e) {
                e.printStackTrace();
            }
            jsonString = gson.toJson(repoMap);
            SharedPreferences.Editor editor = prefs.edit();
            editor.putString(key, jsonString);
            editor.apply();
        }
    }

    public static List<Repo> getSortedList() {
        ArrayList<Repo> list = new ArrayList<>(repoMap.values());
        Collections.sort(list, new Utils.ModuleComparator());
        return list;
    }

    public interface TaskDelegate {
        void taskCompletionResult(String result);
    }

}
