package com.topjohnwu.magisk.widget;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.ColorStateList;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.widget.TintTypedArray;
import androidx.cardview.widget.CardView;

import rikka.layoutinflater.view.LayoutInflaterFactory;

public class Pre23CardViewBackgroundColorFixLayoutInflaterListener implements LayoutInflaterFactory.OnViewCreatedListener {

    private final static Pre23CardViewBackgroundColorFixLayoutInflaterListener INSTANCE = new Pre23CardViewBackgroundColorFixLayoutInflaterListener();

    public static Pre23CardViewBackgroundColorFixLayoutInflaterListener getInstance() {
        return INSTANCE;
    }

    @SuppressLint("RestrictedApi")
    @Override
    public void onViewCreated(@NonNull View view, @Nullable View parent, @NonNull String name, @NonNull Context context, @NonNull AttributeSet attrs) {
        if (!(view instanceof CardView)) {
            return;
        }

        TintTypedArray a = TintTypedArray.obtainStyledAttributes(context, attrs, androidx.cardview.R.styleable.CardView);
        if (a.hasValue(androidx.cardview.R.styleable.CardView_cardBackgroundColor)) {
            ColorStateList colorStateList = a.getColorStateList(androidx.cardview.R.styleable.CardView_cardBackgroundColor);
            if (colorStateList != null) {
                ((CardView) view).setCardBackgroundColor(colorStateList);
            }
        }
        a.recycle();
    }
}

