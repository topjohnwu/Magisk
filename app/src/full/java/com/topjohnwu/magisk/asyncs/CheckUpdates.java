package com.topjohnwu.magisk.asyncs;

import android.os.AsyncTask;

import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.utils.NotificationMgr;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONException;
import org.json.JSONObject;

public class CheckUpdates {

    private static int getInt(JSONObject json, String name, int defValue) {
        if (json == null)
            return defValue;
        try {
            return json.getInt(name);
        } catch (JSONException e) {
            return defValue;
        }
    }

    private static String getString(JSONObject json, String name, String defValue) {
        if (json == null)
            return defValue;
        try {
            return json.getString(name);
        } catch (JSONException e) {
            return defValue;
        }
    }

    private static JSONObject getJson(JSONObject json, String name) {
        try {
            return json.getJSONObject(name);
        } catch (JSONException e) {
            return null;
        }
    }

    public static void fetchUpdates() {
        String jsonStr = "";
        switch (Data.updateChannel) {
            case Const.Value.STABLE_CHANNEL:
                jsonStr = WebService.getString(Const.Url.STABLE_URL);
                break;
            case Const.Value.BETA_CHANNEL:
                jsonStr = WebService.getString(Const.Url.BETA_URL);
                break;
            case Const.Value.CUSTOM_CHANNEL:
                jsonStr = WebService.getString(Data.MM().prefs.getString(Const.Key.CUSTOM_CHANNEL, ""));
                break;
        }

        JSONObject json;
        try {
            json = new JSONObject(jsonStr);
        } catch (JSONException e) {
            return;
        }

        JSONObject magisk = getJson(json, "magisk");
        Data.remoteMagiskVersionString = getString(magisk, "version", null);
        Data.remoteMagiskVersionCode = getInt(magisk, "versionCode", -1);
        Data.magiskLink = getString(magisk, "link", null);
        Data.magiskNoteLink = getString(magisk, "note", null);
        Data.magiskMD5 = getString(magisk, "md5", null);

        JSONObject manager = getJson(json, "app");
        Data.remoteManagerVersionString = getString(manager, "version", null);
        Data.remoteManagerVersionCode = getInt(manager, "versionCode", -1);
        Data.managerLink = getString(manager, "link", null);
        Data.managerNoteLink = getString(manager, "note", null);

        JSONObject uninstaller = getJson(json, "uninstaller");
        Data.uninstallerLink = getString(uninstaller, "link", null);

        JSONObject snet = getJson(json, "snet");
        Data.snetVersionCode = getInt(snet, "versionCode", -1);
        Data.snetLink = getString(snet, "link", null);
    }

    public static void check(Runnable cb) {
        AsyncTask.THREAD_POOL_EXECUTOR.execute(() -> {
            fetchUpdates();
            if (cb != null) {
                if (BuildConfig.VERSION_CODE < Data.remoteManagerVersionCode) {
                    NotificationMgr.managerUpdate();
                } else if (Data.magiskVersionCode < Data.remoteMagiskVersionCode) {
                    NotificationMgr.magiskUpdate();
                }
                cb.run();
            }
            Topic.publish(Topic.UPDATE_CHECK_DONE);
        });
    }

    public static void check() {
        check(null);
    }
}
