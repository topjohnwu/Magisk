package com.topjohnwu.magisk.asyncs;

import android.content.Context;

import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONException;
import org.json.JSONObject;

public class CheckUpdates extends ParallelTask<Void, Void, Void> {

    private static final String UPDATE_JSON = "https://raw.githubusercontent.com/topjohnwu/MagiskManager/update/magisk_update.json";


    private boolean showNotification = false;

    public CheckUpdates(Context context, boolean b) {
        this(context);
        showNotification = b;
    }

    public CheckUpdates(Context context) {
        magiskManager = Utils.getMagiskManager(context);
    }

    @Override
    protected Void doInBackground(Void... voids) {
        String jsonStr = WebService.request(UPDATE_JSON, WebService.GET);
        try {
            JSONObject json = new JSONObject(jsonStr);
            JSONObject magisk = json.getJSONObject("magisk");
            magiskManager.remoteMagiskVersionString = magisk.getString("version");
            magiskManager.remoteMagiskVersionCode = magisk.getInt("versionCode");
            magiskManager.magiskLink = magisk.getString("link");
            magiskManager.releaseNoteLink = magisk.getString("note");
            JSONObject manager = json.getJSONObject("app");
            magiskManager.remoteManagerVersionString = manager.getString("version");
            magiskManager.remoteManagerVersionCode = manager.getInt("versionCode");
            magiskManager.managerLink = manager.getString("link");
        } catch (JSONException ignored) {}
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        if (showNotification && magiskManager.updateNotification) {
            if (magiskManager.magiskVersionCode < magiskManager.remoteMagiskVersionCode) {
                Utils.showMagiskUpdate(magiskManager);
            }
            if (BuildConfig.VERSION_CODE < magiskManager.remoteManagerVersionCode) {
                Utils.showManagerUpdate(magiskManager);
            }
        }
        magiskManager.updateCheckDone.trigger();
        super.onPostExecute(v);
    }
}
