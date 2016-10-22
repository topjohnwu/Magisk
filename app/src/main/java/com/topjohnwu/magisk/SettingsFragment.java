package com.topjohnwu.magisk;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncTask;
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
import android.widget.Toast;

import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;

import butterknife.ButterKnife;

public class SettingsFragment extends PreferenceFragment implements SharedPreferences.OnSharedPreferenceChangeListener {
    private ListPreference themePreference;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.uisettings);
        PreferenceManager.setDefaultValues(getActivity(), R.xml.uisettings, false);
    }

    @Override
    public void onResume() {
        super.onResume();
        getActivity().setTitle(R.string.settings);
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
        themePreference = (ListPreference) findPreference("theme");
        CheckBoxPreference busyboxPreference = (CheckBoxPreference) findPreference("busybox");
        CheckBoxPreference quickTilePreference = (CheckBoxPreference) findPreference("enable_quicktile");
        busyboxPreference.setChecked(Utils.commandExists("unzip"));
        PreferenceManager.getDefaultSharedPreferences(getActivity()).registerOnSharedPreferenceChangeListener(this);
        CheckBoxPreference keepRootOffPreference = (CheckBoxPreference) findPreference("keep_root_off");
        CheckBoxPreference hideRootNotificationPreference = (CheckBoxPreference) findPreference("hide_root_notification");
        themePreference.setSummary(themePreference.getValue());
        if (MagiskFragment.magiskVersion == -1) {
            quickTilePreference.setEnabled(false);
            keepRootOffPreference.setEnabled(false);
            hideRootNotificationPreference.setEnabled(false);
            busyboxPreference.setEnabled(false);
        } else {
            quickTilePreference.setEnabled(true);
            keepRootOffPreference.setEnabled(true);
            hideRootNotificationPreference.setEnabled(true);
            busyboxPreference.setEnabled(true);
        }

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
        Logger.dev("Settings: NewValue is " + key);

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

            Logger.dev("SettingsFragment: theme is " + pref);

        } else if (key.equals("enable_quicktile")) {
            boolean checked = sharedPreferences.getBoolean("enable_quicktile", false);
            if (checked) {
                new AsyncTask<Void, Void, Boolean> () {
                    @Override
                    protected Boolean doInBackground(Void... voids) {
                        return Utils.installTile(getActivity());
                    }
                    @Override
                    protected void onPostExecute(Boolean result) {
                        super.onPostExecute(result);
                        if (result) {
                            Toast.makeText(getActivity(), "Tile installed", Toast.LENGTH_SHORT).show();
                        } else {
                            Toast.makeText(getActivity(), "Tile installation error", Toast.LENGTH_SHORT).show();
                        }
                    }
                }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
            } else {
                new AsyncTask<Void, Void, Boolean> () {
                    @Override
                    protected Boolean doInBackground(Void... voids) {
                        return Utils.uninstallTile(getActivity());
                    }
                    @Override
                    protected void onPostExecute(Boolean result) {
                        super.onPostExecute(result);
                        if (result) {
                            Toast.makeText(getActivity(), "Tile uninstalled", Toast.LENGTH_SHORT).show();
                        } else {
                            Toast.makeText(getActivity(), "Tile uninstallation error", Toast.LENGTH_SHORT).show();
                        }
                    }
                }.executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
            }
        } else if (key.equals("busybox")) {
            boolean checked = sharedPreferences.getBoolean("busybox", false);
            new Async.LinkBusyBox(checked).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
        } else if (key.equals("developer_logging")) {
            Logger.devLog = sharedPreferences.getBoolean("developer_logging", false);
        } else if (key.equals("shell_logging")) {
            Logger.logShell = sharedPreferences.getBoolean("shell_logging", false);
        }

    }
}