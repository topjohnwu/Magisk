package com.topjohnwu.magisk.widget;

import android.content.Context;
import android.util.AttributeSet;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import androidx.annotation.NonNull;

import com.google.android.material.card.MaterialCardView;

public class CheckableMaterialCardView extends MaterialCardView {
    private boolean checkable;
    private boolean checked;
    public CheckableMaterialCardView(Context context) {
        super(context);
    }

    public CheckableMaterialCardView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public CheckableMaterialCardView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override public void setCheckable(boolean checkable) {
        this.checkable = checkable;
    }

    @Override public void setChecked(boolean checked) {
        this.checked = checked;
    }

    @Override public CharSequence getAccessibilityClassName() {
        return CheckableMaterialCardView.class.getName();
    }

    @Override public void onInitializeAccessibilityNodeInfo(@NonNull AccessibilityNodeInfo info) {
        super.onInitializeAccessibilityNodeInfo(info);
        info.setCheckable(checkable);
        info.setChecked(checked);
    }

    @Override public void onInitializeAccessibilityEvent(@NonNull AccessibilityEvent accessibilityEvent) {
        accessibilityEvent.setChecked(checked);
        super.onInitializeAccessibilityEvent(accessibilityEvent);
    }
}
