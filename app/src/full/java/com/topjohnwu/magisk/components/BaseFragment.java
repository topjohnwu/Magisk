package com.topjohnwu.magisk.components;

import android.content.Intent;

import com.topjohnwu.magisk.Data;
import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.utils.Topic;

import androidx.fragment.app.Fragment;
import butterknife.Unbinder;

public class BaseFragment extends Fragment implements Topic.AutoSubscriber {

    public MagiskManager mm;
    protected Unbinder unbinder = null;

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
    public void onDestroyView() {
        super.onDestroyView();
        if (unbinder != null)
            unbinder.unbind();
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
        return BaseActivity.EMPTY_INT_ARRAY;
    }
}
