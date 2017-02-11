package com.topjohnwu.magisk.components;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Utils;

public class Fragment extends android.support.v4.app.Fragment {

    public MagiskManager getApplication() {
        return Utils.getMagiskManager(getActivity());
    }

}
