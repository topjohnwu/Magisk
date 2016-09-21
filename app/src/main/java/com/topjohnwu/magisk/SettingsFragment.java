package com.topjohnwu.magisk;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import com.topjohnwu.magisk.utils.Utils;

import butterknife.ButterKnife;


public class SettingsFragment extends PreferenceFragment {
    private CheckBoxPreference quickTilePreference;
    private CheckBoxPreference devLogPreference;
    private CheckBoxPreference keepRootOffPreference;
    private CheckBoxPreference hideRootNotificationPreference;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);

        // Load the preferences from an XML resource
        addPreferencesFromResource(R.xml.uisettings);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = super.onCreateView(inflater, container, savedInstanceState);
        ButterKnife.bind(this, view);
        quickTilePreference = (CheckBoxPreference) findPreference("enable_quicktile");
        devLogPreference = (CheckBoxPreference) findPreference("developer_logging");
        keepRootOffPreference = (CheckBoxPreference) findPreference("keep_root_off");
        hideRootNotificationPreference = (CheckBoxPreference) findPreference("hide_root_notification");

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
            if (preference == devLogPreference) {
                boolean isChecked = devLogPreference.isChecked();

            }
            if (preference == keepRootOffPreference) {
                boolean isChecked = devLogPreference.isChecked();

            }
            if (preference == hideRootNotificationPreference) {
                boolean isChecked = devLogPreference.isChecked();

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