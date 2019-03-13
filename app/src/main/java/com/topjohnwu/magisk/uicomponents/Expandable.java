package com.topjohnwu.magisk.uicomponents;

public abstract class Expandable {

    private boolean mExpanded = false;

    public final boolean isExpanded() {
        return mExpanded;
    }

    public final void setExpanded(boolean expanded) {
        mExpanded = expanded;
        onSetExpanded(expanded);
    }

    public final void expand() {
        if (mExpanded)
            return;
        onExpand();
        mExpanded = true;
    }

    public final void collapse() {
        if (!mExpanded)
            return;
        onCollapse();
        mExpanded = false;
    }

    protected abstract void onExpand();

    protected abstract void onCollapse();

    protected void onSetExpanded(boolean expanded) {
        if (expanded)
            onExpand();
        else
            onCollapse();
    }
}
