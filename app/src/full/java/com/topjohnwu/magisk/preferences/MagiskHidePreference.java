/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.topjohnwu.magisk.preferences;

import android.content.Context;
import androidx.annotation.Nullable;
import androidx.preference.Preference;
import androidx.preference.PreferenceViewHolder;
import android.util.AttributeSet;
import android.view.View;
import android.widget.Switch;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.fragments.MagiskHideFragment;
import com.topjohnwu.superuser.Shell;

public class MagiskHidePreference extends Preference {

    private Switch mSwitch;
    private boolean mChecked;

    private View mDivider;

    public MagiskHidePreference(Context context, AttributeSet attrs,
                                int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        init();
    }

    public MagiskHidePreference(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    public MagiskHidePreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public MagiskHidePreference(Context context) {
        super(context);
        init();
    }

    private void init() {
        setLayoutResource(R.layout.preference_two_target);
        setWidgetLayoutResource(R.layout.preference_widget_master_switch);
    }

    @Override
    protected void onSetInitialValue(@Nullable Object defaultValue) {
        if (defaultValue == null) {
            defaultValue = false;
        }
        setChecked(getPersistedBoolean((Boolean) defaultValue));
    }

    @Override
    public void onBindViewHolder(PreferenceViewHolder holder) {
        super.onBindViewHolder(holder);
        final View widgetView = holder.findViewById(android.R.id.widget_frame);
        if (widgetView != null) {
            widgetView.setOnClickListener(v -> {
                if (mSwitch != null && !mSwitch.isEnabled()) {
                    return;
                }
                setChecked(!mChecked);
                if (!callChangeListener(mChecked)) {
                    setChecked(!mChecked);
                } else {
                    persistBoolean(mChecked);
                }
            });
        }

        mSwitch = (Switch) holder.findViewById(R.id.switchWidget);
        mSwitch.setContentDescription(getTitle());
        mSwitch.setChecked(mChecked);

        mDivider = holder.findViewById(R.id.two_target_divider);
        setHasFragment(Shell.rootAccess() && mChecked);
    }

    public boolean isChecked() {
        return mSwitch != null && mChecked;
    }

    public void setChecked(boolean checked) {
        mChecked = checked;
        if (mSwitch != null) {
            mSwitch.setChecked(checked);
        }
        setHasFragment(Shell.rootAccess() && mChecked);
        persistBoolean(mChecked);
    }

    private void setHasFragment(boolean hasFragment) {
        if (mDivider != null) {
            mDivider.setVisibility(hasFragment ? View.VISIBLE : View.GONE);
        }
        setFragment(hasFragment ? MagiskHideFragment.class.getName() : null);
    }

    public Switch getSwitch() {
        return mSwitch;
    }

    @Override
    protected void performClick(View view) {
        super.performClick(view);

        if (!mChecked) {
            setChecked(true);
        }
    }
}