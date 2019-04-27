package com.topjohnwu.magisk.ui.base;

import android.content.Intent;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.utils.Event;

import androidx.fragment.app.Fragment;
import butterknife.Unbinder;

/**
 * @deprecated This class is not to be used. It is marked for deletion.
 */
@Deprecated
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
        startActivityForResult(intent, requestCode, (resultCode, data) ->
                onActivityResult(requestCode, resultCode, data));
    }

    public void startActivityForResult(Intent intent, int requestCode,
                                       BaseActivity.ActivityResultListener listener) {
        ((IBaseLeanback) requireActivity()).startActivityForResult(intent, requestCode, listener);
    }

    protected void runWithExternalRW(Runnable callback) {
        ((IBaseLeanback) requireActivity()).runWithExternalRW(callback);
    }

    @Override
    public int[] getListeningEvents() {
        return BaseActivity.EMPTY_INT_ARRAY;
    }

    @Override
    public void onEvent(int event) {}
}
