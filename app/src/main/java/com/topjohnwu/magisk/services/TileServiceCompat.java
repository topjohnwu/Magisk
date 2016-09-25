package com.topjohnwu.magisk.services;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;

import com.kcoppock.broadcasttilesupport.BroadcastTileIntentBuilder;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Utils;

public class TileServiceCompat extends Service {
    private static BroadcastReceiver clickTileReceiver;

    private static boolean root, autoRoot;

    public static final String TILE_ID = "Magisk";
    public static final String ACTION_TILE_CLICK = "magisk.ACTION_TILE_CLICK";
    public static final String EXTRA_CLICK_TYPE = "magisk.EXTRA_CLICK_TYPE";
    public static final int CLICK_TYPE_UNKNOWN = -1;
    public static final int CLICK_TYPE_SIMPLE = 0;
    public static final int CLICK_TYPE_LONG = 1;

    public TileServiceCompat() {
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        root = true;
        registerClickTileReceiver();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        updateRoots();
        updateTile();
        return super.onStartCommand(intent, flags, startId);
    }

    private void updateRoots() {
        root = Utils.rootEnabled();
        autoRoot = Utils.autoToggleEnabled(getApplicationContext());
    }


    private void registerClickTileReceiver() {
        clickTileReceiver = new BroadcastReceiver() {

            @Override
            public void onReceive(Context context, Intent intent) {
                int clickType = intent.getIntExtra(EXTRA_CLICK_TYPE, CLICK_TYPE_UNKNOWN);
                switch (clickType) {
                    case CLICK_TYPE_SIMPLE:
                        onSimpleClick();
                        break;
                    case CLICK_TYPE_LONG:
                        onLongClick();
                        break;
                }
            }
        };
        registerReceiver(clickTileReceiver, new IntentFilter(ACTION_TILE_CLICK));
    }


    private void onSimpleClick() {
		updateRoots();
        updateTile();
        if (autoRoot) {

            Utils.toggleAutoRoot(false, getApplicationContext());
            if (!Utils.hasServicePermission(getApplicationContext())) {
                Intent it = new Intent(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
                sendBroadcast(it);
            }
        } else {
            Utils.toggleRoot(!root, getApplicationContext());
        }
    }

    private void onLongClick() {
        updateRoots();
        updateTile();
        Utils.toggleAutoRoot(!autoRoot,getApplicationContext());
        if (!Utils.hasServicePermission(getApplicationContext())) {
            Intent it = new Intent(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
            sendBroadcast(it);
        }
    }

    private void updateTile() {
        BroadcastTileIntentBuilder broadcastTileIntentBuilder = new BroadcastTileIntentBuilder(this, TILE_ID);
        if (autoRoot) {
            broadcastTileIntentBuilder.setLabel(getApplicationContext().getString(R.string.auto_toggle));
            broadcastTileIntentBuilder.setIconResource(R.drawable.ic_autoroot_white);

        } else {
            if (root) {
                broadcastTileIntentBuilder.setLabel("Root enabled");
                broadcastTileIntentBuilder.setIconResource(R.drawable.root_white);

            } else {
                broadcastTileIntentBuilder.setLabel("Root disabled");
                broadcastTileIntentBuilder.setIconResource(R.drawable.root_grey);

            }
        }

        Intent simpleClick = new Intent(ACTION_TILE_CLICK);
        simpleClick.putExtra(EXTRA_CLICK_TYPE, CLICK_TYPE_SIMPLE);
        Intent longClick = new Intent(ACTION_TILE_CLICK);
        longClick.putExtra(EXTRA_CLICK_TYPE, CLICK_TYPE_LONG);

        broadcastTileIntentBuilder.setVisible(true);
        broadcastTileIntentBuilder.setOnClickBroadcast(simpleClick);
        broadcastTileIntentBuilder.setOnLongClickBroadcast(longClick);
        this.sendBroadcast(broadcastTileIntentBuilder.build());

    }


    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(clickTileReceiver);
    }

}
