package com.topjohnwu.magisk.asyncs;

import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.Global;
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
        MagiskManager mm = Global.MM();
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
        Global.remoteMagiskVersionString = getString(magisk, "version", null);
        Global.remoteMagiskVersionCode = getInt(magisk, "versionCode", -1);
        Global.magiskLink = getString(magisk, "link", null);
        Global.magiskNoteLink = getString(magisk, "note", null);

        JSONObject manager = getJson(json, "app");
        Global.remoteManagerVersionString = getString(manager, "version", null);
        Global.remoteManagerVersionCode = getInt(manager, "versionCode", -1);
        Global.managerLink = getString(manager, "link", null);
        Global.managerNoteLink = getString(manager, "note", null);

        JSONObject uninstaller = getJson(json, "uninstaller");
        Global.uninstallerLink = getString(uninstaller, "link", null);

        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        MagiskManager mm = Global.MM();
        if (showNotification) {
            if (BuildConfig.VERSION_CODE < Global.remoteManagerVersionCode) {
                NotificationMgr.managerUpdate();
            } else if (Global.magiskVersionCode < Global.remoteMagiskVersionCode) {
                NotificationMgr.magiskUpdate();
            }
        }
        mm.updateCheckDone.publish();
        super.onPostExecute(v);
    }
}
