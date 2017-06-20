package com.topjohnwu.magisk.components;

import android.animation.ValueAnimator;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.LinearLayout;

public abstract class ExpandableViewHolder extends RecyclerView.ViewHolder {

    protected ViewGroup expandLayout;
    private ValueAnimator expandAnimator, collapseAnimator;
    private static int expandHeight = 0;

    public boolean mExpanded = false;

    public ExpandableViewHolder(View itemView) {
        super(itemView);
        setExpandLayout(itemView);
        expandLayout.getViewTreeObserver().addOnPreDrawListener(
                new ViewTreeObserver.OnPreDrawListener() {

                    @Override
                    public boolean onPreDraw() {
                        if (expandHeight == 0) {
                            final int widthSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
                            final int heightSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
                            expandLayout.measure(widthSpec, heightSpec);
                            expandHeight = expandLayout.getMeasuredHeight();
                        }

                        expandLayout.getViewTreeObserver().removeOnPreDrawListener(this);
                        expandLayout.setVisibility(View.GONE);
                        expandAnimator = slideAnimator(0, expandHeight);
                        collapseAnimator = slideAnimator(expandHeight, 0);
                        return true;
                    }

                });
    }

    public void setExpanded(boolean expanded) {
        mExpanded = expanded;
        ViewGroup.LayoutParams layoutParams = expandLayout.getLayoutParams();
        layoutParams.height = expanded ? expandHeight : 0;
        expandLayout.setLayoutParams(layoutParams);
        expandLayout.setVisibility(expanded ? View.VISIBLE : View.GONE);
    }

    public void expand() {
        if (mExpanded) return;
        expandLayout.setVisibility(View.VISIBLE);
        expandAnimator.start();
        mExpanded = true;
    }

    public void collapse() {
        if (!mExpanded) return;
        collapseAnimator.start();
        mExpanded = false;
    }

    public abstract void setExpandLayout(View itemView);

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
