package com.topjohnwu.core.tasks;

import com.topjohnwu.core.App;
import com.topjohnwu.core.Const;
import com.topjohnwu.core.Data;
import com.topjohnwu.core.utils.Topic;
import com.topjohnwu.net.Networking;
import com.topjohnwu.net.Request;
import com.topjohnwu.net.ResponseListener;

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

    private static Request getRequest() {
        String url;
        switch (Data.updateChannel) {
            case Const.Value.BETA_CHANNEL:
                url = Const.Url.BETA_URL;
                break;
            case Const.Value.CUSTOM_CHANNEL:
                url = App.self.prefs.getString(Const.Key.CUSTOM_CHANNEL, "");
                break;
            case Const.Value.STABLE_CHANNEL:
            default:
                url = Const.Url.STABLE_URL;
                break;
        }
        return Networking.get(url);
    }

    public static void check() {
        getRequest().getAsJSONObject(new UpdateListener(null));
    }

    public static void checkNow(Runnable cb) {
        JSONObject json = getRequest().execForJSONObject().getResult();
        if (json != null)
            new UpdateListener(cb).onResponse(json);
    }

    private static class UpdateListener implements ResponseListener<JSONObject> {

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

            Topic.publish(Topic.UPDATE_CHECK_DONE);

            if (cb != null)
                cb.run();
        }
    }
}
