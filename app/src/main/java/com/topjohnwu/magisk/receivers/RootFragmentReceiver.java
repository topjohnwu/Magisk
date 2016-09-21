package com.topjohnwu.magisk.receivers;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class RootFragmentReceiver extends BroadcastReceiver {

    private Receiver mFragment;
    public RootFragmentReceiver(Receiver fragment) {
        mFragment = fragment;
    }

    public void onReceive(Context context, Intent intent) {
        if (intent.getAction().equals(420)) {
            mFragment.onResult();
        }
    }
}
