package com.topjohnwu.magisk.asyncs;

import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.NotificationMgr;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONException;
import org.json.JSONObject;

public class CheckUpdates extends ParallelTask<Void, Void, Void> {

    private boolean showNotification;

    public CheckUpdates() {
        this(false);
    }

    public CheckUpdates(boolean b) {
        showNotification = b;
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
    protected Void doInBackground(Void... voids) {
        MagiskManager mm = Data.MM();
        String jsonStr = "";
        switch (Data.updateChannel) {
            case Const.Value.STABLE_CHANNEL:
                jsonStr = WebService.getString(Const.Url.STABLE_URL);
                break;
            case Const.Value.BETA_CHANNEL:
                jsonStr = WebService.getString(Const.Url.BETA_URL);
                break;
            case Const.Value.CUSTOM_CHANNEL:
                jsonStr = WebService.getString(mm.prefs.getString(Const.Key.CUSTOM_CHANNEL, ""));
                break;
        }

        JSONObject json;
        try {
            json = new JSONObject(jsonStr);
        } catch (JSONException e) {
            return null;
        }

        JSONObject magisk = getJson(json, "magisk");
        Data.remoteMagiskVersionString = getString(magisk, "version", null);
        Data.remoteMagiskVersionCode = getInt(magisk, "versionCode", -1);
        Data.magiskLink = getString(magisk, "link", null);
        Data.magiskNoteLink = getString(magisk, "note", null);

        JSONObject manager = getJson(json, "app");
        Data.remoteManagerVersionString = getString(manager, "version", null);
        Data.remoteManagerVersionCode = getInt(manager, "versionCode", -1);
        Data.managerLink = getString(manager, "link", null);
        Data.managerNoteLink = getString(manager, "note", null);

        JSONObject uninstaller = getJson(json, "uninstaller");
        Data.uninstallerLink = getString(uninstaller, "link", null);

        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        if (showNotification) {
            if (BuildConfig.VERSION_CODE < Data.remoteManagerVersionCode) {
                NotificationMgr.managerUpdate();
            } else if (Data.magiskVersionCode < Data.remoteMagiskVersionCode) {
                NotificationMgr.magiskUpdate();
            }
        }
        Topic.publish(Topic.UPDATE_CHECK_DONE);
        super.onPostExecute(v);
    }
}
