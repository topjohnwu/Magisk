package com.topjohnwu.magisk.components;

import com.topjohnwu.magisk.MagiskManager;

public class Fragment extends android.support.v4.app.Fragment {

    public MagiskManager getApplication() {
        return (MagiskManager) getActivity().getApplicationContext();
    }

}
