/*
 * Copyright 2016 dvdandroid
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.topjohnwu.magisk.components;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.topjohnwu.magisk.R;

/**
 * @author dvdandroid
 */
public class AboutCardRow extends LinearLayout {

    private final String title;
    private final Drawable icon;

    private final TextView mTitle;
    private final TextView mSummary;
    private final ImageView mIcon;

    private final View mView;

    public AboutCardRow(Context context) {
        this(context, null);
    }

    public AboutCardRow(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public AboutCardRow(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        LayoutInflater.from(context).inflate(R.layout.info_item_row, this);
        TypedArray a = context.getTheme().obtainStyledAttributes(attrs, R.styleable.AboutCardRow, 0, 0);

        try {
            title = a.getString(R.styleable.AboutCardRow_text);
            icon = a.getDrawable(R.styleable.AboutCardRow_icon);
        } finally {
            a.recycle();
        }

        mView = findViewById(R.id.container);

        mTitle = (TextView) findViewById(android.R.id.title);
        mSummary = (TextView) findViewById(android.R.id.summary);
        mIcon = (ImageView) findViewById(android.R.id.icon);

        mTitle.setText(title);
        mIcon.setImageDrawable(icon);
    }

    @Override
    public void setOnClickListener(OnClickListener l) {
        super.setOnClickListener(l);

        mView.setOnClickListener(l);
    }

    public void setSummary(String s) {
        mSummary.setText(s);
    }

    public void removeSummary() {
        mSummary.setVisibility(GONE);
    }
}