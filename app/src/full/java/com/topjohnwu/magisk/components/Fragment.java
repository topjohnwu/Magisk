package com.topjohnwu.magisk.components;

import android.content.Intent;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;

public class Fragment extends android.support.v4.app.Fragment {

    public MagiskManager getApplication() {
        return Utils.getMagiskManager(getActivity());
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
    public void startActivityForResult(Intent intent, int requestCode) {
        startActivityForResult(intent, requestCode, this::onActivityResult);
    }

    public void startActivityForResult(Intent intent, int requestCode, Activity.ActivityResultListener listener) {
        ((Activity) getActivity()).startActivityForResult(intent, requestCode, listener);
    }

    public void runWithPermission(String[] permissions, Runnable callback) {
        Activity.runWithPermission(getActivity(), permissions, callback);
    }
}
