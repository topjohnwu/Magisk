package com.topjohnwu.magisk.tile;

import android.content.Context;
import android.content.SharedPreferences;
import android.support.annotation.NonNull;

/**
 * Helper class for tracking preference values to keep track of the state of the custom tile
 */
final public class TilePreferenceHelper {
    private static final String PREFS_NAME = "tile_prefs";

    private final SharedPreferences mSharedPreferences;

    TilePreferenceHelper(@NonNull Context context) {
        mSharedPreferences = context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE);
    }

    void setBoolean(@NonNull String key, boolean value) {
        mSharedPreferences.edit().putBoolean(key, value).apply();
    }

    boolean getBoolean(@NonNull String key) {
        return mSharedPreferences.getBoolean(key, false);
    }
}