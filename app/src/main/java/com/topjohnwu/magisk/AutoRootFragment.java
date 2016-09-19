package com.topjohnwu.magisk;

import android.app.ProgressDialog;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.v4.app.ListFragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

import com.topjohnwu.magisk.utils.ApplicationAdapter;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class AutoRootFragment extends ListFragment {
    private PackageManager packageManager = null;
    private List<ApplicationInfo> applist = null;
    private ApplicationAdapter listadaptor = null;
    public ListView listView;
    public SharedPreferences prefs;
    List<String> arrayBlackList;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.auto_root_fragment, container, false);
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        listView = getListView();
        packageManager = getActivity().getPackageManager();
        prefs = PreferenceManager.getDefaultSharedPreferences(getContext());
        if (!prefs.contains("auto_blacklist")) {
            SharedPreferences.Editor editor = prefs.edit();
            Set<String> set = new HashSet<>();
            set.add("com.google.android.apps.walletnfcrel");
            set.add("com.google.android.gms");
            editor.putStringSet("auto_blacklist", set);
            editor.apply();
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

        Set<String> blackListSet = prefs.getStringSet("auto_blacklist", null);


        assert blackListSet != null;
        arrayBlackList = new ArrayList<>(blackListSet);

        if (!arrayBlackList.contains(appToCheck)) {
            arrayBlackList.add(appToCheck);

        } else {
            for (int i = 0; i < arrayBlackList.size(); i++) {
                if (appToCheck.equals(arrayBlackList.get(i))) {
                    arrayBlackList.remove(i);
                }
            }

        }
        prefs.edit().putStringSet("auto_blacklist", new HashSet<>(arrayBlackList)).apply();
        listadaptor.UpdateRootStatusView(position, v);

    }

    private List<ApplicationInfo> checkForLaunchIntent(List<ApplicationInfo> list) {
        ArrayList<ApplicationInfo> applist = new ArrayList<>();
        for (ApplicationInfo info : list) {
            try {
                if (null != packageManager.getLaunchIntentForPackage(info.packageName)) {
                    applist.add(info);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        Collections.sort(applist, new CustomComparator());

        return applist;
    }

    public class CustomComparator implements Comparator<ApplicationInfo> {
        @Override
        public int compare(ApplicationInfo o1, ApplicationInfo o2) {
            packageManager = getActivity().getPackageManager();
            return o1.loadLabel(packageManager).toString().compareTo(o2.loadLabel(packageManager).toString());
        }
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