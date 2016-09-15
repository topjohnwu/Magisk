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
    List<String> arrayBlackList,arrayWhiteList;

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
            editor.putStringSet("auto_blacklist", set);
            set.clear();
            set.add("com.kermidas.TitaniumBackupPro");
            editor.putStringSet("auto_whitelist",set);
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
        Set<String> whiteListSet = prefs.getStringSet("auto_whitelist", null);

        assert blackListSet != null;
        arrayBlackList = new ArrayList<>(blackListSet);
        assert whiteListSet != null;
        arrayWhiteList = new ArrayList<>(whiteListSet);
        Log.d("Magisk", "Trying to toggle for " + appToCheck + " stringset is " + arrayBlackList.toString());

        if ((!arrayBlackList.contains(appToCheck)) && (!arrayWhiteList.contains(appToCheck))) {
            Log.d("Magisk", "App is not in any array, adding to whitelist");
            arrayWhiteList.add(appToCheck);


        } else if (arrayWhiteList.contains(appToCheck)) {
            Log.d("Magisk", "App is in whitelist, moving to blacklist");
            for (int i = 0; i < arrayWhiteList.size(); i++) {
                if (appToCheck.equals(arrayWhiteList.get(i))) {
                    arrayWhiteList.remove(i);
                }
            }
            arrayBlackList.add(appToCheck);

        } else if (arrayBlackList.contains(appToCheck)) {
            Log.d("Magisk", "App is in Blacklist, removing");
            for (int i = 0; i < arrayBlackList.size(); i++) {
                if (appToCheck.equals(arrayBlackList.get(i))) {
                    arrayBlackList.remove(i);
                }
            }

        }
        Set<String> set2 = new HashSet<>(arrayBlackList);
        Log.d("Magisk", "Committing set, value is: " + set2);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putStringSet("auto_blacklist", new HashSet<>(arrayBlackList));
        editor.putStringSet("auto_whitelist", new HashSet<>(arrayWhiteList));
        editor.apply();
        listadaptor.UpdateRootStatusView(position,v);


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