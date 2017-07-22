package com.topjohnwu.magisk.components;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.CallbackEvent;
import com.topjohnwu.magisk.utils.Utils;

public class Fragment extends android.support.v4.app.Fragment {

    public MagiskManager getApplication() {
        return Utils.getMagiskManager(getActivity());
    }

    @Override
    public void onResume() {
        super.onResume();
        if (this instanceof CallbackEvent.Listener) {
            ((CallbackEvent.Listener) this).registerEvents();
        }
    }

    @Override
    public void onPause() {
        if (this instanceof CallbackEvent.Listener) {
            ((CallbackEvent.Listener) this).unregisterEvents();
        }
        super.onPause();
    }
}
