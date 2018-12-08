package com.topjohnwu.magisk.asyncs;

import com.androidnetworking.AndroidNetworking;
import com.androidnetworking.error.ANError;
import com.androidnetworking.interfaces.JSONObjectRequestListener;
import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.components.Notifications;
import com.topjohnwu.magisk.utils.Topic;

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

    public static void check(Runnable cb) {
        String url;
        switch (Data.updateChannel) {
            case Const.Value.STABLE_CHANNEL:
                url = Const.Url.STABLE_URL;
                break;
            case Const.Value.BETA_CHANNEL:
                url = Const.Url.BETA_URL;
                break;
            case Const.Value.CUSTOM_CHANNEL:
                url = Data.MM().prefs.getString(Const.Key.CUSTOM_CHANNEL, "");
                break;
            default:
                return;
        }
        AndroidNetworking.get(url).build().getAsJSONObject(new UpdateListener(cb));
    }

    public static void check() {
        check(null);
    }

    private static class UpdateListener implements JSONObjectRequestListener {

        private Runnable cb;

        UpdateListener(Runnable callback) {
            cb = callback;
        }

        @Override
        public void onResponse(JSONObject json) {
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

            if (cb != null) {
                if (BuildConfig.VERSION_CODE < Data.remoteManagerVersionCode) {
                    Notifications.managerUpdate();
                } else if (Data.magiskVersionCode < Data.remoteMagiskVersionCode) {
                    Notifications.magiskUpdate();
                }
                cb.run();
            }
            Topic.publish(Topic.UPDATE_CHECK_DONE);
        }

        @Override
        public void onError(ANError anError) {}
    }
}
