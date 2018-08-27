package com.topjohnwu.snet;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.res.AssetManager;
import android.content.res.Resources;

public class SwapResContext extends ContextWrapper {

    private AssetManager asset;
    private Resources resources;

    public SwapResContext(Context base, String apk) {
        super(base);
        try {
            asset = AssetManager.class.newInstance();
            AssetManager.class.getMethod("addAssetPath", String.class).invoke(asset, apk);
        } catch (Exception e) {
            e.printStackTrace();
        }
        Resources res = base.getResources();
        resources = new Resources(asset, res.getDisplayMetrics(), res.getConfiguration());
    }

    @Override
    public Resources getResources() {
        return resources;
    }

    @Override
    public AssetManager getAssets() {
        return asset;
    }
}
