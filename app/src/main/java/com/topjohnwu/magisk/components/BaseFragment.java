package com.topjohnwu.magisk.components;

import android.content.Intent;

import androidx.fragment.app.Fragment;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.utils.Event;

import butterknife.Unbinder;

public abstract class BaseFragment extends Fragment implements Event.AutoListener {

    public App app = App.self;
    protected Unbinder unbinder = null;

    @Override
    public void onResume() {
        super.onResume();
        Event.register(this);
    }

    @Override
    public void onPause() {
        Event.unregister(this);
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
    public int[] getListeningEvents() {
        return BaseActivity.EMPTY_INT_ARRAY;
    }

    @Override
    public void onEvent(int event) {}
}
