package com.topjohnwu.magisk.widget;

import android.animation.Animator;
import android.animation.ObjectAnimator;
import android.animation.StateListAnimator;
import android.content.Context;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.AttributeSet;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.customview.view.AbsSavedState;
import androidx.interpolator.view.animation.FastOutLinearInInterpolator;

import com.google.android.material.bottomnavigation.BottomNavigationView;
import com.topjohnwu.magisk.R;

public class ConcealableBottomNavigationView extends BottomNavigationView {

    private static final int[] STATE_SET = {
            R.attr.state_hidden
    };

    private boolean isHidden;
    public ConcealableBottomNavigationView(@NonNull Context context) {
        this(context, null);
    }

    public ConcealableBottomNavigationView(@NonNull Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, com.google.android.material.R.attr.bottomNavigationStyle);
    }

    public ConcealableBottomNavigationView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, com.google.android.material.R.style.Widget_Design_BottomNavigationView);
    }

    public ConcealableBottomNavigationView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    private void recreateAnimator(int height) {
        Animator toHidden = ObjectAnimator.ofFloat(this, "translationY", height);
        toHidden.setDuration(175);
        toHidden.setInterpolator(new FastOutLinearInInterpolator());
        Animator toUnhidden = ObjectAnimator.ofFloat(this, "translationY", 0);
        toUnhidden.setDuration(225);
        toUnhidden.setInterpolator(new FastOutLinearInInterpolator());

        StateListAnimator animator = new StateListAnimator();

        animator.addState(STATE_SET, toHidden);
        animator.addState(new int[]{}, toUnhidden);

        setStateListAnimator(animator);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);

        recreateAnimator(getMeasuredHeight());
    }

    @Override
    protected int[] onCreateDrawableState(int extraSpace) {
        final int[] drawableState = super.onCreateDrawableState(extraSpace + 1);
        if (isHidden()) {
            mergeDrawableStates(drawableState, STATE_SET);
        }
        return drawableState;
    }

    public boolean isHidden() {
        return isHidden;
    }

    public void setHidden(boolean raised) {
        if (isHidden != raised) {
            isHidden = raised;
            refreshDrawableState();
        }
    }

    @NonNull
    @Override
    protected Parcelable onSaveInstanceState() {
        SavedState state = new SavedState(super.onSaveInstanceState());
        state.isHidden = isHidden();
        return state;
    }

    @Override
    protected void onRestoreInstanceState(Parcelable state) {
        final SavedState ss = (SavedState) state;
        super.onRestoreInstanceState(ss.getSuperState());

        if (ss.isHidden) {
            setHidden(isHidden);
        }
    }

    static class SavedState extends AbsSavedState {

        public boolean isHidden;

        public SavedState(Parcel source) {
            super(source, ConcealableBottomNavigationView.class.getClassLoader());
            isHidden = source.readByte() != 0;
        }

        public SavedState(Parcelable superState) {
            super(superState);
        }

        @Override
        public void writeToParcel(Parcel out, int flags) {
            super.writeToParcel(out, flags);
            out.writeByte(isHidden ? (byte) 1 : (byte) 0);
        }

        public static final Creator<SavedState> CREATOR = new Creator<>() {

            @Override
            public SavedState createFromParcel(Parcel source) {
                return new SavedState(source);
            }

            @Override
            public SavedState[] newArray(int size) {
                return new SavedState[size];
            }
        };
    }
}
