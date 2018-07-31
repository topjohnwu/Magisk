package com.topjohnwu.magisk.components;

import android.content.Intent;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;

public class Fragment extends android.support.v4.app.Fragment implements Topic.AutoSubscriber {

    public MagiskManager getApplication() {
        return Utils.getMagiskManager(getActivity());
    }

    @Override
    public void onResume() {
        super.onResume();
        Topic.subscribe(this);
    }

    @Override
    public void onPause() {
        Topic.unsubscribe(this);
        super.onPause();
    }

    @Override
    public void startActivityForResult(Intent intent, int requestCode) {
        startActivityForResult(intent, requestCode, this::onActivityResult);
    }

    public void startActivityForResult(Intent intent, int requestCode, Activity.ActivityResultListener listener) {
        ((Activity) requireActivity()).startActivityForResult(intent, requestCode, listener);
    }

    public void runWithPermission(String[] permissions, Runnable callback) {
        ((Activity) requireActivity()).runWithPermission(permissions,callback);
    }

    @Override
    public int[] getSubscribedTopics() {
        return FlavorActivity.EMPTY_INT_ARRAY;
    }
}
