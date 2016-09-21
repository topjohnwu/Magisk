package com.topjohnwu.magisk.services;

import android.annotation.SuppressLint;
import android.graphics.drawable.Icon;
import android.service.quicksettings.Tile;
import android.service.quicksettings.TileService;
import android.util.Log;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;

@SuppressLint("NewApi")
public class QuickSettingTileService extends TileService {
    private int STATE_CURRENT;

    public QuickSettingTileService() {
    }

    @Override
    public void onTileAdded() {
        super.onTileAdded();
        setupState();
    }

    @Override
    public void onClick() {
        switchState();

    }

    private void setupState() {
        Icon iconRoot = Icon.createWithResource(getApplicationContext(), R.drawable.root);
        Icon iconAuto = Icon.createWithResource(getApplicationContext(), R.drawable.ic_autoroot);
        Tile tile = this.getQsTile();
        boolean autoRootStatus = Utils.autoRootEnabled(getApplicationContext());
        boolean rootStatus = Utils.rootEnabled();
        Log.d("Magisk", "QST: Auto and root are " + autoRootStatus + " and " + rootStatus);
        if (autoRootStatus) {
            tile.setLabel("Auto-root");
            tile.setIcon(iconAuto);
            tile.setState(Tile.STATE_ACTIVE);
            STATE_CURRENT = 0;
        } else {
            if (rootStatus) {
                tile.setLabel("Root enabled");
                tile.setIcon(iconRoot);
                tile.setState(Tile.STATE_ACTIVE);
                STATE_CURRENT = 1;

            } else {
                tile.setLabel("Root disabled");
                tile.setIcon(iconRoot);
                tile.setState(Tile.STATE_INACTIVE);
                STATE_CURRENT = 2;

            }
        }
        tile.updateTile();
    }

    private void switchState() {
        Log.d("Magisk", "QST: Switching state to " + STATE_CURRENT);
        switch (STATE_CURRENT) {
            case 0:
                Utils.toggleRoot(false);
                Utils.toggleAutoRoot(false, getApplicationContext());
                break;
            case 1:
                Utils.toggleAutoRoot(true, getApplicationContext());
                break;
            case 2:
                Utils.toggleRoot(true);
                break;
        }
        setupState();

    }
}