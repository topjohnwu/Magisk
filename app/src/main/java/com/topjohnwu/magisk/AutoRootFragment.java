package com.topjohnwu.magisk;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.v4.app.ListFragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

import com.topjohnwu.magisk.utils.ApplicationAdapter;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class AutoRootFragment extends ListFragment {
    private PackageManager packageManager = null;
    private List<ApplicationInfo> applist = null;
    private ApplicationAdapter listadaptor = null;
    public ListView listView;
    public SharedPreferences prefs;
    private View view;
    List<String> arrayList;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.auto_root_fragment, container, false);
        return view;
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        listView = getListView();
        packageManager = getActivity().getPackageManager();
        prefs = PreferenceManager.getDefaultSharedPreferences(getContext());
        if (!prefs.contains("autoapps")) {
            SharedPreferences.Editor editor = prefs.edit();
            Set<String> set = new HashSet<String>();
            set.add("com.google.android.apps.walletnfcrel");
            editor.putStringSet("autoapps", set);
            editor.commit();
        }
        new LoadApplications().execute();
    }

    @Override
    public void onResume() {
        super.onResume();
        new LoadApplications().execute();

    }


    @Override
    public void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);
        ApplicationInfo app = applist.get(position);
        ToggleApp(app.packageName, position, v);

    }

    private void ToggleApp(String appToCheck, int position, View v) {

        Set<String> set = prefs.getStringSet("autoapps", null);

        arrayList = new ArrayList<>(set);
        Log.d("Magisk", "Trying to toggle for " + appToCheck + " stringset is " + arrayList.toString());
        SharedPreferences.Editor editor = prefs.edit();

        if (arrayList.contains(appToCheck)) {
            Log.d("Magisk", "App is in array, removing");

            for (int i = 0; i < arrayList.size(); i++) {
                if (appToCheck.equals(arrayList.get(i))) {
                    arrayList.remove(i);
                }
            }

        } else {
            arrayList.add(appToCheck);

        }
        Set<String> set2 = new HashSet<String>(arrayList);
        Log.d("Magisk", "Committing set, value is: " + set2);
        editor.putStringSet("autoapps", set2);
        editor.apply();
        listadaptor.UpdateRootStatusView(position,v);


    }

    private List<ApplicationInfo> checkForLaunchIntent(List<ApplicationInfo> list) {
        ArrayList<ApplicationInfo> applist = new ArrayList<ApplicationInfo>();
        for (ApplicationInfo info : list) {
            try {
                if (null != packageManager.getLaunchIntentForPackage(info.packageName)) {
                    applist.add(info);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        return applist;
    }

    private class LoadApplications extends AsyncTask<Void, Void, Void> {
        private ProgressDialog progress = null;

        @Override
        protected Void doInBackground(Void... params) {
            applist = checkForLaunchIntent(packageManager.getInstalledApplications(PackageManager.GET_META_DATA));
            listadaptor = new ApplicationAdapter(getActivity(),
                    R.layout.app_list_row, applist);

            return null;
        }

        @Override
        protected void onCancelled() {
            super.onCancelled();
        }

        @Override
        protected void onPostExecute(Void result) {
            setListAdapter(listadaptor);
            progress.dismiss();
            super.onPostExecute(result);
        }

        @Override
        protected void onPreExecute() {
            progress = ProgressDialog.show(getActivity(), null,
                    "Loading application info...");
            super.onPreExecute();
        }

        @Override
        protected void onProgressUpdate(Void... values) {
            super.onProgressUpdate(values);
        }
    }
}