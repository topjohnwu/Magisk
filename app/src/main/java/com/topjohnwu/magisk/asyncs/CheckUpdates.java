package com.topjohnwu.magisk.asyncs;

import android.content.Context;

import com.topjohnwu.magisk.BuildConfig;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONException;
import org.json.JSONObject;

public class CheckUpdates extends ParallelTask<Void, Void, Void> {

    public static final int STABLE_CHANNEL = 0;
    public static final int BETA_CHANNEL = 1;

    private static final String STABLE_URL = "https://raw.githubusercontent.com/topjohnwu/MagiskManager/update/stable.json";
    private static final String BETA_URL = "https://raw.githubusercontent.com/topjohnwu/MagiskManager/update/beta.json";

    private boolean showNotification = false;

    public CheckUpdates(Context context) {
        super(context);
    }

    public CheckUpdates(Context context, boolean b) {
        super(context);
        showNotification = b;
    }

    @Override
    protected Void doInBackground(Void... voids) {
        MagiskManager mm = getMagiskManager();
        if (mm == null) return null;
        String jsonStr;
        switch (mm.updateChannel) {
            case STABLE_CHANNEL:
                jsonStr = WebService.getString(STABLE_URL);
                break;
            case BETA_CHANNEL:
                jsonStr = WebService.getString(BETA_URL);
                break;
            default:
                jsonStr = null;
        }
        try {
            JSONObject json = new JSONObject(jsonStr);
            JSONObject magisk = json.getJSONObject("magisk");
            mm.remoteMagiskVersionString = magisk.getString("version");
            mm.remoteMagiskVersionCode = magisk.getInt("versionCode");
            mm.magiskLink = magisk.getString("link");
            mm.releaseNoteLink = magisk.getString("note");
            JSONObject manager = json.getJSONObject("app");
            mm.remoteManagerVersionString = manager.getString("version");
            mm.remoteManagerVersionCode = manager.getInt("versionCode");
            mm.managerLink = manager.getString("link");
        } catch (JSONException ignored) {}
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        MagiskManager mm = getMagiskManager();
        if (mm == null) return;
        if (showNotification && mm.updateNotification) {
            if (BuildConfig.VERSION_CODE < mm.remoteManagerVersionCode) {
                Utils.showManagerUpdateNotification(mm);
            } else if (mm.magiskVersionCode < mm.remoteMagiskVersionCode) {
                Utils.showMagiskUpdateNotification(mm);
            }
        }
        mm.updateCheckDone.publish();
        super.onPostExecute(v);
    }
}
