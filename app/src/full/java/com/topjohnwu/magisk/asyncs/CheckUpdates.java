package com.topjohnwu.magisk.asyncs;

import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.ShowUI;
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
        try {
            JSONObject json = new JSONObject(jsonStr);
            JSONObject magisk = json.getJSONObject("magisk");
            mm.remoteMagiskVersionString = magisk.getString("version");
            mm.remoteMagiskVersionCode = magisk.getInt("versionCode");
            mm.magiskLink = magisk.getString("link");
            mm.magiskNoteLink = magisk.getString("note");
            JSONObject manager = json.getJSONObject("app");
            mm.remoteManagerVersionString = manager.getString("version");
            mm.remoteManagerVersionCode = manager.getInt("versionCode");
            mm.managerLink = manager.getString("link");
            mm.managerNoteLink = manager.getString("note");
            JSONObject uninstaller = json.getJSONObject("uninstaller");
            mm.uninstallerLink = uninstaller.getString("link");
        } catch (JSONException ignored) {}
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        MagiskManager mm = MagiskManager.get();
        if (showNotification) {
            if (BuildConfig.VERSION_CODE < mm.remoteManagerVersionCode) {
                ShowUI.managerUpdateNotification();
            } else if (mm.magiskVersionCode < mm.remoteMagiskVersionCode) {
                ShowUI.magiskUpdateNotification();
            }
        }
        mm.updateCheckDone.publish();
        super.onPostExecute(v);
    }
}
