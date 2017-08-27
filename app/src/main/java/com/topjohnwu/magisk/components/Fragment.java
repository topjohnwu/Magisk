package com.topjohnwu.magisk.components;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;

public class Fragment extends android.support.v4.app.Fragment {

    public MagiskManager getApplication() {
        return Utils.getMagiskManager(getActivity());
    }

    public Shell getShell() {
        return Shell.getShell(getActivity());
    }

    @Override
    public void onResume() {
        super.onResume();
        if (this instanceof Topic.Subscriber) {
            ((Topic.Subscriber) this).subscribeTopics();
        }
    }

    @Override
    public void onPause() {
        if (this instanceof Topic.Subscriber) {
            ((Topic.Subscriber) this).unsubscribeTopics();
        }
        super.onPause();
    }
}
