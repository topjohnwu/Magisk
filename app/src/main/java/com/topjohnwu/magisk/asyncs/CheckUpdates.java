package com.topjohnwu.magisk.asyncs;

import android.app.Activity;

import com.topjohnwu.magisk.utils.WebService;

import org.json.JSONException;
import org.json.JSONObject;

public class CheckUpdates extends ParallelTask<Void, Void, Void> {

    public static final String UPDATE_JSON = "https://raw.githubusercontent.com/topjohnwu/MagiskManager/updates/magisk_update.json";

    public CheckUpdates(Activity context) {
        super(context);
    }

    @Override
    protected Void doInBackground(Void... voids) {
        String jsonStr = WebService.request(UPDATE_JSON, WebService.GET);
        try {
            JSONObject json = new JSONObject(jsonStr);
            JSONObject magisk = json.getJSONObject("magisk");
            magiskManager.remoteMagiskVersion = magisk.getDouble("versionCode");
            magiskManager.magiskLink = magisk.getString("link");
            magiskManager.releaseNoteLink = magisk.getString("note");
        } catch (JSONException ignored) {
        }
        return null;
    }

    @Override
    protected void onPostExecute(Void v) {
        magiskManager.updateCheckDone.trigger();
    }
}
