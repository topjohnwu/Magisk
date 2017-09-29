package com.topjohnwu.magisk.container;

import android.support.v7.widget.RecyclerView;

import java.util.ArrayList;

public class AdaptiveList<E> extends ArrayList<E> {

    private Runnable callback;
    private RecyclerView mView;

    public AdaptiveList(RecyclerView v) {
        mView = v;
    }

    public void updateView() {
        mView.getAdapter().notifyDataSetChanged();
        mView.scrollToPosition(mView.getAdapter().getItemCount() - 1);
    }

    public void setCallback(Runnable cb) {
        callback = cb;
    }

    public boolean add(E e) {
        boolean ret = super.add(e);
        if (ret) {
            if (callback == null) {
                updateView();
            } else {
                callback.run();
            }
        }
        return ret;
    }
}
