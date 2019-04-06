package com.topjohnwu.magisk.tasks;

import android.os.SystemClock;

import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.utils.Event;
import com.topjohnwu.net.Networking;
import com.topjohnwu.net.Request;
import com.topjohnwu.net.ResponseListener;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.internal.UiThreadHandler;

import org.json.JSONException;
import org.json.JSONObject;

public class CheckUpdates {

    private static Request getRequest() {
        String url;
        switch ((int) Config.get(Config.Key.UPDATE_CHANNEL)) {
            case Config.Value.BETA_CHANNEL:
                url = Const.Url.BETA_URL;
                break;
            case Config.Value.CUSTOM_CHANNEL:
                url = Config.get(Config.Key.CUSTOM_CHANNEL);
                break;
            case Config.Value.CANARY_CHANNEL:
                url = Const.Url.CANARY_URL;
                break;
            case Config.Value.CANARY_DEBUG_CHANNEL:
                url = Const.Url.CANARY_DEBUG_URL;
                break;
            default:
                url = Const.Url.STABLE_URL;
                break;
        }
        return Networking.get(url);
    }

    public static void check() {
        check(null);
    }

    public static void check(Runnable cb) {
        Request request = getRequest();
        UpdateListener listener = new UpdateListener(cb);
        if (ShellUtils.onMainThread()) {
            request.getAsJSONObject(listener);
        } else {
            JSONObject json = request.execForJSONObject().getResult();
            if (json != null)
                listener.onResponse(json);
        }
    }

    private static class UpdateListener implements ResponseListener<JSONObject> {

        private Runnable cb;
        private long start;

        UpdateListener(Runnable callback) {
            cb = callback;
            start = SystemClock.uptimeMillis();
        }

        private int getInt(JSONObject json, String name, int defValue) {
            if (json == null)
                return defValue;
            try {
                return json.getInt(name);
            } catch (JSONException e) {
                return defValue;
            }
        }

        private String getString(JSONObject json, String name, String defValue) {
            if (json == null)
                return defValue;
            try {
                return json.getString(name);
            } catch (JSONException e) {
                return defValue;
            }
        }

        private JSONObject getJson(JSONObject json, String name) {
            try {
                return json.getJSONObject(name);
            } catch (JSONException e) {
                return null;
            }
        }

        @Override
        public void onResponse(JSONObject json) {
            JSONObject magisk = getJson(json, "magisk");
            Config.remoteMagiskVersionCode = getInt(magisk, "versionCode", -1);

            if ((int) Config.get(Config.Key.UPDATE_CHANNEL) == Config.Value.DEFAULT_CHANNEL) {
                if (Config.magiskVersionCode > Config.remoteMagiskVersionCode) {
                    // If we are newer than current stable channel, switch to beta
                    Config.set(Config.Key.UPDATE_CHANNEL, Config.Value.BETA_CHANNEL);
                    check(cb);
                    return;
                } else {
                    Config.set(Config.Key.UPDATE_CHANNEL, Config.Value.STABLE_CHANNEL);
                }
            }

            Config.remoteMagiskVersionString = getString(magisk, "version", null);
            Config.magiskLink = getString(magisk, "link", null);
            Config.magiskNoteLink = getString(magisk, "note", null);
            Config.magiskMD5 = getString(magisk, "md5", null);

            JSONObject manager = getJson(json, "app");
            Config.remoteManagerVersionString = getString(manager, "version", null);
            Config.remoteManagerVersionCode = getInt(manager, "versionCode", -1);
            Config.managerLink = getString(manager, "link", null);
            Config.managerNoteLink = getString(manager, "note", null);

            JSONObject uninstaller = getJson(json, "uninstaller");
            Config.uninstallerLink = getString(uninstaller, "link", null);

            UiThreadHandler.handler.postAtTime(() -> Event.trigger(Event.UPDATE_CHECK_DONE),
                    start + 1000  /* Add artificial delay to let UI behave correctly */);

            if (cb != null)
                cb.run();
        }
    }
}
