package com.topjohnwu.magisk.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.preference.PreferenceManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.R;

import java.util.List;
import java.util.Set;

public class ApplicationAdapter extends ArrayAdapter<ApplicationInfo> {
    private List<ApplicationInfo> appsList = null;
    private Context context;
    private PackageManager packageManager;
    public SharedPreferences prefs;

    public ApplicationAdapter(Context context, int textViewResourceId,
                              List<ApplicationInfo> appsList) {
        super(context, textViewResourceId, appsList);
        this.context = context;
        this.appsList = appsList;
        packageManager = context.getPackageManager();
    }

    @Override
    public int getCount() {
        return ((null != appsList) ? appsList.size() : 0);
    }

    @Override
    public ApplicationInfo getItem(int position) {
        return ((null != appsList) ? appsList.get(position) : null);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }


    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View view = convertView;

        prefs = PreferenceManager.getDefaultSharedPreferences(getContext());
        if (null == view) {
            LayoutInflater layoutInflater = (LayoutInflater) context
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            view = layoutInflater.inflate(R.layout.list_item_app, null);
        }

        ApplicationInfo applicationInfo = appsList.get(position);
        if (null != applicationInfo) {
            TextView appName = (TextView) view.findViewById(R.id.app_name);
            TextView packageName = (TextView) view.findViewById(R.id.app_paackage);
            ImageView iconview = (ImageView) view.findViewById(R.id.app_icon);
            CheckBox statusview = (CheckBox) view.findViewById(R.id.checkbox);
            appName.setText(applicationInfo.loadLabel(packageManager));
            packageName.setText(applicationInfo.packageName);
            iconview.setImageDrawable(applicationInfo.loadIcon(packageManager));
            if (CheckApp(applicationInfo.packageName)) {
                statusview.setChecked(true);
            } else {
                statusview.setChecked(false);
            }
        }
        return view;
    }

    public void UpdateRootStatusView(int position, View convertView) {
        View view = convertView;
        if (null == view) {
            LayoutInflater layoutInflater = (LayoutInflater) context
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            view = layoutInflater.inflate(R.layout.list_item_app, null);
        }
        ApplicationInfo applicationInfo = appsList.get(position);
        if (null != applicationInfo) {
            CheckBox statusview = (CheckBox) view.findViewById(R.id.checkbox);
            if (CheckApp(applicationInfo.packageName)) {
                statusview.setChecked(true);
            } else {
                statusview.setChecked(false);
            }
        }

    }

    private boolean CheckApp(String appToCheck) {
        boolean starter = false;
        Set<String> set = prefs.getStringSet("auto_blacklist", null);
        if (set != null) {
            for (String string : set) {
                if (string.equals(appToCheck)) {
                    starter = true;
                }
            }
        }

        return starter;

    }

}