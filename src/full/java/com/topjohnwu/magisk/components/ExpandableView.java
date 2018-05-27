package com.topjohnwu.magisk.components;

import android.animation.ValueAnimator;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;

public interface ExpandableView {

    class Container {
        public ViewGroup expandLayout;
        ValueAnimator expandAnimator, collapseAnimator;
        boolean mExpanded = false;
        int expandHeight = 0;
    }

    // Provide state info
    Container getContainer();

    default void setupExpandable() {
        Container container = getContainer();
        container.expandLayout.getViewTreeObserver().addOnPreDrawListener(
            new ViewTreeObserver.OnPreDrawListener() {

                @Override
                public boolean onPreDraw() {
                    if (container.expandHeight == 0) {
                        final int widthSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
                        final int heightSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
                        container.expandLayout.measure(widthSpec, heightSpec);
                        container.expandHeight = container.expandLayout.getMeasuredHeight();
                    }

                    container.expandLayout.getViewTreeObserver().removeOnPreDrawListener(this);
                    container.expandLayout.setVisibility(View.GONE);
                    container.expandAnimator = slideAnimator(0, container.expandHeight);
                    container.collapseAnimator = slideAnimator(container.expandHeight, 0);
                    return true;
                }

            });
    }

    default boolean isExpanded() {
        return getContainer().mExpanded;
    }

    default void setExpanded(boolean expanded) {
        Container container = getContainer();
        container.mExpanded = expanded;
        ViewGroup.LayoutParams layoutParams = container.expandLayout.getLayoutParams();
        layoutParams.height = expanded ? container.expandHeight : 0;
        container.expandLayout.setLayoutParams(layoutParams);
        container.expandLayout.setVisibility(expanded ? View.VISIBLE : View.GONE);
    }

    default void expand() {
        Container container = getContainer();
        if (container.mExpanded) return;
        container.expandLayout.setVisibility(View.VISIBLE);
        container.expandAnimator.start();
        container.mExpanded = true;
    }

    default void collapse() {
        Container container = getContainer();
        if (!container.mExpanded) return;
        container.collapseAnimator.start();
        container.mExpanded = false;
    }

    default ValueAnimator slideAnimator(int start, int end) {
        Container container = getContainer();
        ValueAnimator animator = ValueAnimator.ofInt(start, end);

        animator.addUpdateListener(valueAnimator -> {
            int value = (Integer) valueAnimator.getAnimatedValue();
            ViewGroup.LayoutParams layoutParams = container.expandLayout.getLayoutParams();
            layoutParams.height = value;
            container.expandLayout.setLayoutParams(layoutParams);
        });
        return animator;
    }
}
