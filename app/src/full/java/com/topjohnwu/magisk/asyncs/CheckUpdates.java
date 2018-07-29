package com.topjohnwu.magisk.asyncs;

import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.NotificationMgr;
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
        MagiskManager mm = MagiskManager.get();
        String jsonStr = "";
        switch (mm.updateChannel) {
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
        mm.remoteMagiskVersionString = getString(magisk, "version", null);
        mm.remoteMagiskVersionCode = getInt(magisk, "versionCode", -1);
        mm.magiskLink = getString(magisk, "link", null);
        mm.magiskNoteLink = getString(magisk, "note", null);

        JSONObject manager = getJson(json, "app");
        mm.remoteManagerVersionString = getString(manager, "version", null);
        mm.remoteManagerVersionCode = getInt(manager, "versionCode", -1);
        mm.managerLink = getString(manager, "link", null);
        mm.managerNoteLink = getString(manager, "note", null);

        JSONObject uninstaller = getJson(json, "uninstaller");
        mm.uninstallerLink = getString(uninstaller, "link", null);

        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        MagiskManager mm = MagiskManager.get();
        if (showNotification) {
            if (BuildConfig.VERSION_CODE < mm.remoteManagerVersionCode) {
                NotificationMgr.managerUpdate();
            } else if (mm.magiskVersionCode < mm.remoteMagiskVersionCode) {
                NotificationMgr.magiskUpdate();
            }
        }
        mm.updateCheckDone.publish();
        super.onPostExecute(v);
    }
}
