package com.topjohnwu.magisk.components;

import android.content.Intent;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;

public class Fragment extends android.support.v4.app.Fragment {

    private ActivityResultListener activityResultListener;

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

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (activityResultListener != null)
            activityResultListener.onActivityResult(requestCode, resultCode, data);
        activityResultListener = null;
    }

    public void startActivityForResult(Intent intent, int requestCode, ActivityResultListener listener) {
        activityResultListener = listener;
        super.startActivityForResult(intent, requestCode);
    }

    public interface ActivityResultListener {
        void onActivityResult(int requestCode, int resultCode, Intent data);
    }
}
