package com.topjohnwu.magisk.components;

import android.content.Intent;
import android.support.v4.app.Fragment;

import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Topic;

public class BaseFragment extends Fragment implements Topic.AutoSubscriber {

    public MagiskManager mm;

    public BaseFragment() {
        mm = Data.MM();
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

    public void startActivityForResult(Intent intent, int requestCode, BaseActivity.ActivityResultListener listener) {
        ((BaseActivity) requireActivity()).startActivityForResult(intent, requestCode, listener);
    }

    public void runWithPermission(String[] permissions, Runnable callback) {
        ((BaseActivity) requireActivity()).runWithPermission(permissions,callback);
    }

    @Override
    public int[] getSubscribedTopics() {
        return FlavorActivity.EMPTY_INT_ARRAY;
    }
}
