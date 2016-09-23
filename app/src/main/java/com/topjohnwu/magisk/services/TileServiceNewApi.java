package com.topjohnwu.magisk.services;

import android.annotation.SuppressLint;
import android.content.SharedPreferences;
import android.graphics.drawable.Icon;
import android.preference.PreferenceManager;
import android.service.quicksettings.Tile;
import android.util.Log;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;

@SuppressLint("NewApi")
public class TileServiceNewApi extends android.service.quicksettings.TileService implements
        SharedPreferences.OnSharedPreferenceChangeListener {
    private int STATE_CURRENT;

    public TileServiceNewApi() {
    }



    @Override
    public void onTileAdded() {
        super.onTileAdded();
        setupState();
        this.getQsTile().updateTile();
    }

    @Override
    public void onClick() {
        switchState();
        this.getQsTile().updateTile();

    }

    private void setupState() {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        preferences.registerOnSharedPreferenceChangeListener(this);
        Logger.dh("TileService(New): SetupState");
        Icon iconRoot = Icon.createWithResource(getApplicationContext(), R.drawable.root);
        Icon iconAuto = Icon.createWithResource(getApplicationContext(), R.drawable.ic_autoroot);
        Tile tile = this.getQsTile();
        boolean autoRootStatus = Utils.autoToggleEnabled(getApplicationContext());
        boolean rootStatus = Utils.rootEnabled();
        int rootsStatus = Utils.CheckRootsState(getApplicationContext());
        Log.d("Magisk", "QST: Auto and root are " + autoRootStatus + " and " + rootStatus + Utils.CheckRootsState(getApplicationContext()));
        if (rootsStatus == 2) {
            tile.setLabel(getApplicationContext().getString(R.string.auto_toggle));
            tile.setIcon(iconAuto);
            tile.setState(Tile.STATE_ACTIVE);

        } else if (rootsStatus == 1) {
            tile.setLabel("Root enabled");
            tile.setIcon(iconRoot);
            tile.setState(Tile.STATE_ACTIVE);


        } else {
            tile.setLabel("Root disabled");
            tile.setIcon(iconRoot);
            tile.setState(Tile.STATE_INACTIVE);


        }

        tile.updateTile();
    }

    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,String key)
    {
        Logger.dh("TileService: Key Change registered for " + key);
        if (key.equals("autoRootEnable")) {

        }
    }

    private void switchState() {
        switch (Utils.CheckRootsState(getApplicationContext())) {
            case 2:
                Utils.toggleRoot(true);
                Utils.toggleAutoRoot(false, getApplicationContext());
                break;
            case 1:
                Utils.toggleRoot(false);
                break;
            case 0:
                Utils.toggleAutoRoot(true, getApplicationContext());
                break;
        }
        setupState();

    }
}