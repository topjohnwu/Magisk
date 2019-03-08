package com.topjohnwu.magisk.uicomponents;

import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.RotateAnimation;

public class ArrowExpandedViewHolder extends ExpandableViewHolder {

    private View arrow;

    public ArrowExpandedViewHolder(ViewGroup viewGroup, View arrow) {
        super(viewGroup);
        this.arrow = arrow;
    }

    @Override
    public void setExpanded(boolean expanded) {
        super.setExpanded(expanded);
        if (arrow != null)
            arrow.setRotation(expanded ? 180 : 0);
    }

    @Override
    public void expand() {
        super.expand();
        setRotate(new RotateAnimation(0, 180,
                Animation.RELATIVE_TO_SELF, 0.5f, Animation.RELATIVE_TO_SELF, 0.5f));
    }

    @Override
    public void collapse() {
        super.collapse();
        setRotate(new RotateAnimation(180, 0,
                Animation.RELATIVE_TO_SELF, 0.5f, Animation.RELATIVE_TO_SELF, 0.5f));
    }

    private void setRotate(RotateAnimation rotate) {
        rotate.setDuration(300);
        rotate.setFillAfter(true);
        arrow.startAnimation(rotate);
    }
}
