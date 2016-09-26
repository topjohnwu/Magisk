package com.topjohnwu.magisk;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;

import butterknife.ButterKnife;

public class SettingsFragment extends PreferenceFragment implements SharedPreferences.OnSharedPreferenceChangeListener {
    private CheckBoxPreference quickTilePreference;
    private ListPreference themePreference;

    public SettingsFragment() {
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.uisettings);
        PreferenceManager.setDefaultValues(getActivity(), R.xml.uisettings, false);
    }

    @Override
    public void onResume() {
        super.onResume();
        getActivity().setTitle("Settings");
        PreferenceManager.getDefaultSharedPreferences(getActivity()).registerOnSharedPreferenceChangeListener(this);

    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        PreferenceManager.getDefaultSharedPreferences(getActivity()).unregisterOnSharedPreferenceChangeListener(this);

    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = super.onCreateView(inflater, container, savedInstanceState);
        ButterKnife.bind(this, view);
        quickTilePreference = (CheckBoxPreference) findPreference("enable_quicktile");
        themePreference = (ListPreference) findPreference("theme");
        PreferenceManager.getDefaultSharedPreferences(getActivity()).registerOnSharedPreferenceChangeListener(this);
        CheckBoxPreference keepRootOffPreference = (CheckBoxPreference) findPreference("keep_root_off");
        CheckBoxPreference hideRootNotificationPreference = (CheckBoxPreference) findPreference("hide_root_notification");
        themePreference.setSummary(themePreference.getValue());
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

    @Override
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
        Logger.dh("Settings: NewValue is " + key);

        if (key.equals("theme")) {
            String pref = sharedPreferences.getString(key, "");

            themePreference.setSummary(pref);
            if (pref.equals("Dark")) {
                getActivity().getApplication().setTheme(R.style.AppTheme_dh);
            } else {
                getActivity().getApplication().setTheme(R.style.AppTheme);
            }
            Intent intent = new Intent(getActivity(), MainActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.putExtra("Relaunch", "Settings");
            startActivity(intent);

            Logger.dh("SettingsFragment: theme is " + pref);

        }

    }
}