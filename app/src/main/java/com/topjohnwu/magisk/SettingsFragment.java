package com.topjohnwu.magisk;

import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.topjohnwu.magisk.utils.Utils;

import butterknife.ButterKnife;


public class SettingsFragment extends PreferenceFragment {
    private CheckBoxPreference quickTilePreference;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);

        // Load the preferences from an XML resource
        addPreferencesFromResource(R.xml.uisettings);
    }

    @Override
    public void onResume() {
        super.onResume();
        getActivity().setTitle("Settings");
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = super.onCreateView(inflater, container, savedInstanceState);
        ButterKnife.bind(this, view);
        quickTilePreference = (CheckBoxPreference) findPreference("enable_quicktile");
        CheckBoxPreference keepRootOffPreference = (CheckBoxPreference) findPreference("keep_root_off");
        CheckBoxPreference hideRootNotificationPreference = (CheckBoxPreference) findPreference("hide_root_notification");

        if (Utils.magiskVersion == -1) {
            quickTilePreference.setEnabled(false);
            keepRootOffPreference.setEnabled(false);
            hideRootNotificationPreference.setEnabled(false);
        } else {
            quickTilePreference.setEnabled(true);
            keepRootOffPreference.setEnabled(true);
            hideRootNotificationPreference.setEnabled(true);
        }

        Preference.OnPreferenceClickListener preferenceClickListener = preference -> {
            if (preference == quickTilePreference) {
                boolean isChecked = quickTilePreference.isChecked();
                if (isChecked) {
                    Utils.installTile(getActivity());
                } else {
                    Utils.uninstallTile(getActivity());
                }

            }

            return false;
        };

        quickTilePreference.setOnPreferenceClickListener(preferenceClickListener);
        // calculate margins
        int horizontalMargin = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 2, getResources().getDisplayMetrics());
        int verticalMargin = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 2, getResources().getDisplayMetrics());
        TypedValue tv = new TypedValue();
        int actionBarHeight = 130;
        if (getActivity().getTheme().resolveAttribute(android.R.attr.actionBarSize, tv, true)) {
            actionBarHeight = TypedValue.complexToDimensionPixelSize(tv.data, getResources().getDisplayMetrics());
        }

        view.setPadding(horizontalMargin, actionBarHeight, horizontalMargin, verticalMargin);


        return view;
    }
}