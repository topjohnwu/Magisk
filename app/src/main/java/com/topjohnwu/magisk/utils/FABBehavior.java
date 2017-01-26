package com.topjohnwu.magisk.utils;


import android.content.Context;
import android.support.design.widget.CoordinatorLayout;
import android.support.design.widget.Snackbar;
import android.support.v4.view.ViewCompat;
import android.support.v4.view.ViewPropertyAnimatorCompat;
import android.util.AttributeSet;
import android.view.View;

import com.github.clans.fab.FloatingActionMenu;

import java.util.List;

/**
 * Created by Matteo on 08/08/2015.
 *
 * Floating Action Menu Behavior for Clans.FloatingActionButton
 * https://github.com/Clans/FloatingActionButton/
 *
 * Use this behavior as your app:layout_behavior attribute in your Floating Action Menu to use the
 * FabMenu in a Coordinator Layout.
 *
 * Remember to use the correct namespace for the fab:
 * xmlns:app="http://schemas.android.com/apk/res-auto"
 */
public class FABBehavior extends CoordinatorLayout.Behavior<View> {

    private float mTranslationY;

    public FABBehavior(Context context, AttributeSet attrs) {
        super();
    }

    @Override
    public boolean layoutDependsOn(CoordinatorLayout parent, View child, View dependency) {
        return dependency instanceof Snackbar.SnackbarLayout;
    }

    @Override
    public boolean onDependentViewChanged(CoordinatorLayout parent, View child, View dependency) {
        if (dependency instanceof Snackbar.SnackbarLayout) {
            updateTranslation(parent, child);
        }

        return false;
    }

    @Override
    public void onDependentViewRemoved(CoordinatorLayout parent, View child, View dependency) {
        if (dependency instanceof Snackbar.SnackbarLayout) {
            revertTranslation(child);
        }
    }

    private void updateTranslation(CoordinatorLayout parent, View child) {
        float translationY = getTranslationY(parent, child);
        if (translationY != mTranslationY) {
            ViewPropertyAnimatorCompat anim = ViewCompat.animate(child);
            anim.cancel();
            anim.translationY(translationY).setDuration(100);
            mTranslationY = translationY;
        }
    }

    private void revertTranslation(View child) {
        if (mTranslationY != 0) {
            ViewPropertyAnimatorCompat anim = ViewCompat.animate(child);
            anim.cancel();
            anim.translationY(0).setDuration(100);
            mTranslationY = 0;
        }
    }

    private float getTranslationY(CoordinatorLayout parent, View child) {
        float minOffset = 0.0F;
        List<View> dependencies = parent.getDependencies(child);
        int i = 0;

        for (int z = dependencies.size(); i < z; ++i) {
            View view = dependencies.get(i);
            if (view instanceof Snackbar.SnackbarLayout && parent.doViewsOverlap(child, view)) {
                minOffset = Math.min(minOffset, ViewCompat.getTranslationY(view) - (float) view.getHeight());
            }
        }

        return minOffset;
    }

    /**
     * onStartNestedScroll and onNestedScroll will hide/show the FabMenu when a scroll is detected.
     */
    @Override
    public boolean onStartNestedScroll(CoordinatorLayout coordinatorLayout, View child,
                                       View directTargetChild, View target, int nestedScrollAxes) {
        return nestedScrollAxes == ViewCompat.SCROLL_AXIS_VERTICAL ||
                super.onStartNestedScroll(coordinatorLayout, child, directTargetChild, target,
                        nestedScrollAxes);
    }

    @Override
    public void onNestedScroll(CoordinatorLayout coordinatorLayout, View child, View target,
                               int dxConsumed, int dyConsumed, int dxUnconsumed, int dyUnconsumed) {
        super.onNestedScroll(coordinatorLayout, child, target, dxConsumed, dyConsumed, dxUnconsumed,
                dyUnconsumed);
        FloatingActionMenu fabMenu = (FloatingActionMenu) child;
        if (dyConsumed > 0 && !fabMenu.isMenuButtonHidden()) {
            fabMenu.hideMenuButton(true);
        } else if (dyConsumed < 0 && fabMenu.isMenuButtonHidden()) {
            fabMenu.showMenuButton(true);
        }
    }
}
