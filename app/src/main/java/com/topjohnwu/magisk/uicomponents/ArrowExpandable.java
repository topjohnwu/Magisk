package com.topjohnwu.magisk.uicomponents;

import android.view.View;
import android.view.animation.Animation;
import android.view.animation.RotateAnimation;

public class ArrowExpandable extends Expandable {
    protected Expandable mBase;
    private View arrow;

    public ArrowExpandable(Expandable base, View arrow) {
        mBase = base;
        this.arrow = arrow;
    }

    @Override
    public void onExpand() {
        mBase.onExpand();
        setRotate(new RotateAnimation(0, 180,
                Animation.RELATIVE_TO_SELF, 0.5f, Animation.RELATIVE_TO_SELF, 0.5f));
    }

    @Override
    public void onCollapse() {
        mBase.onCollapse();
        setRotate(new RotateAnimation(180, 0,
                Animation.RELATIVE_TO_SELF, 0.5f, Animation.RELATIVE_TO_SELF, 0.5f));
    }

    @Override
    public void onSetExpanded(boolean expanded) {
        mBase.onSetExpanded(expanded);
        if (arrow != null)
            arrow.setRotation(expanded ? 180 : 0);
    }

    private void setRotate(RotateAnimation rotate) {
        rotate.setDuration(300);
        rotate.setFillAfter(true);
        arrow.startAnimation(rotate);
    }
}
