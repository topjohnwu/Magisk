package com.topjohnwu.magisk.services;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.graphics.drawable.Icon;
import android.preference.PreferenceManager;
import android.service.quicksettings.Tile;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;

@SuppressLint("NewApi")
public class TileServiceNewApi extends android.service.quicksettings.TileService {
    private int mRootsState;

    public TileServiceNewApi() {
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Logger.dev("QST (New): Service start");
        return super.onStartCommand(intent, flags, startId);

    }

    @Override
    public void onTileAdded() {
        super.onTileAdded();
        Logger.dev("QST (New): Tile added");
        setupState();
        this.getQsTile().updateTile();
    }

    @Override
    public void onClick() {
        mRootsState = Utils.CheckRootsState(getApplicationContext());
        switchState(mRootsState);
        Logger.dev("QST (New): Tile clicked");
    }


    @Override
    public void onStartListening() {
        super.onStartListening();
        setupState();
        Logger.dev("QST (New): Tile is listening");
    }

    @Override
    public void onStopListening() {
        super.onStopListening();
        Logger.dev("QST (New): Tile stopped listening");
    }

    private void setupState() {
        if (!Utils.hasServicePermission(getApplicationContext())) {
            PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).edit().putBoolean("autoRootEnable",false).apply();
        }
        mRootsState = Utils.CheckRootsState(getApplicationContext());
        Logger.dev("QST (New): SetupState");
        Icon iconRoot = Icon.createWithResource(getApplicationContext(), R.drawable.root);
        Icon iconAuto = Icon.createWithResource(getApplicationContext(), R.drawable.ic_autoroot);
        Tile tile = getQsTile();
        Logger.dev("QST: State is " + mRootsState);

        switch (mRootsState) {
            case 2:
                tile.setLabel(getApplicationContext().getString(R.string.auto_toggle));
                tile.setIcon(iconAuto);
                tile.setState(Tile.STATE_ACTIVE);
                break;

            case 1:
                tile.setLabel("Root enabled");
                tile.setIcon(iconRoot);
                tile.setState(Tile.STATE_ACTIVE);
                break;

            default:
                tile.setLabel("Root disabled");
                tile.setIcon(iconRoot);
                tile.setState(Tile.STATE_INACTIVE);
                break;
        }

        tile.updateTile();
    }

    private void switchState(int rootsState) {

        switch (rootsState) {
            case 2:
                Utils.toggleRoot(true, getApplicationContext());
                if (!Utils.hasServicePermission(getApplicationContext())) {
                    Intent it = new Intent(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
                    sendBroadcast(it);
                }
                Utils.toggleAutoRoot(false, getApplicationContext());
                break;
            case 1:
                Utils.toggleRoot(false, getApplicationContext());
                break;
            case 0:
                if (!Utils.hasServicePermission(getApplicationContext())) {
                    Intent it = new Intent(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
                    sendBroadcast(it);
                }
                Utils.toggleAutoRoot(true, getApplicationContext());
                break;
        }
        this.onStartListening();
        setupState();

    }
}