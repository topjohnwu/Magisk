package com.topjohnwu.magisk.uicomponents;

import android.animation.ValueAnimator;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;

public class ExpandableViewHolder extends Expandable {

    private ViewGroup expandLayout;
    private ValueAnimator expandAnimator, collapseAnimator;
    private int expandHeight = 0;

    public ExpandableViewHolder(ViewGroup viewGroup) {
        expandLayout = viewGroup;
        setExpanded(false);
        expandLayout.getViewTreeObserver().addOnPreDrawListener(
                new ViewTreeObserver.OnPreDrawListener() {

                    @Override
                    public boolean onPreDraw() {
                        if (expandHeight == 0) {
                            expandLayout.measure(0, 0);
                            expandHeight = expandLayout.getMeasuredHeight();
                        }

                        expandLayout.getViewTreeObserver().removeOnPreDrawListener(this);
                        expandAnimator = slideAnimator(0, expandHeight);
                        collapseAnimator = slideAnimator(expandHeight, 0);
                        return true;
                    }

                });
    }

    @Override
    protected void onExpand() {
        expandLayout.setVisibility(View.VISIBLE);
        expandAnimator.start();
    }

    @Override
    protected void onCollapse() {
        collapseAnimator.start();
    }

    @Override
    protected void onSetExpanded(boolean expanded) {
        ViewGroup.LayoutParams layoutParams = expandLayout.getLayoutParams();
        layoutParams.height = expanded ? expandHeight : 0;
        expandLayout.setLayoutParams(layoutParams);
    }

    private ValueAnimator slideAnimator(int start, int end) {
        ValueAnimator animator = ValueAnimator.ofInt(start, end);

        animator.addUpdateListener(valueAnimator -> {
            int value = (Integer) valueAnimator.getAnimatedValue();
            ViewGroup.LayoutParams layoutParams = expandLayout.getLayoutParams();
            layoutParams.height = value;
            expandLayout.setLayoutParams(layoutParams);
        });
        return animator;
    }
}
