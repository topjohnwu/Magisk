package com.topjohnwu.magisk;

import android.app.ListFragment;
import android.app.ProgressDialog;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

import com.topjohnwu.magisk.utils.ApplicationAdapter;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import static com.topjohnwu.magisk.utils.Utils.WhichHide;

public class MagiskHideFragment extends ListFragment {
    private PackageManager packageManager = null;
    private List<ApplicationInfo> applist = null;
    private ApplicationAdapter listadaptor = null;
    public ListView listView;
    public SharedPreferences prefs;
    private int hideVersion;
    List<String> arrayBlackList;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
        View view = inflater.inflate(R.layout.auto_root_fragment, container, false);
        int horizontalMargin = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 2, getResources().getDisplayMetrics());
        int verticalMargin = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 2, getResources().getDisplayMetrics());
        TypedValue tv = new TypedValue();
        int actionBarHeight = 130;
        if (getActivity().getTheme().resolveAttribute(android.R.attr.actionBarSize, tv, true)) {
            actionBarHeight = TypedValue.complexToDimensionPixelSize(tv.data, getResources().getDisplayMetrics());
        }

        view.setPadding(horizontalMargin, actionBarHeight, horizontalMargin, verticalMargin);
        hideVersion = WhichHide(getActivity());
        return view;
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);


    }

    @Override
    public void onResume() {
        super.onResume();
        initializeElements();
        super.onResume();
        getActivity().setTitle(R.string.magiskhide);
        hideVersion = WhichHide(getActivity());

    }

    private void initializeElements() {
        listView = getListView();
        packageManager = getActivity().getPackageManager();
        new LoadApplications().execute();
    }

    @Override
    public void onListItemClick(ListView l, View v, int position, long id) {
        Logger.dev("Click");
        super.onListItemClick(l, v, position, id);
        ApplicationInfo app = applist.get(position);
        ToggleApp(app.packageName, position, v);

    }

    private void ToggleApp(String appToCheck, int position, View v) {
        Logger.dev("Magisk", "MagiskHideFragment: ToggleApp called for " + appToCheck);
        Set<String> blackListSet = prefs.getStringSet("auto_blacklist", null);
        assert blackListSet != null;
        arrayBlackList = new ArrayList<>(blackListSet);
        String UID = Utils.getAppUID(appToCheck);
        if (!arrayBlackList.contains(appToCheck)) {
            arrayBlackList.add(appToCheck);
            switch (hideVersion) {
                case 1 :
                    Shell.su("/magisk/.core/magiskhide/add " + appToCheck);
                    break;
                case 2 :
                    Shell.su("/su/suhide/add " + UID);
                    break;
                case 3 :
                    Shell.su("/su/suhide/add " + UID + "&& /magisk/.core/magiskhide/add " + appToCheck);
                    break;
                default :
                    break;

            }



        } else {
            for (int i = 0; i < arrayBlackList.size(); i++) {
                if (appToCheck.equals(arrayBlackList.get(i))) {
                    arrayBlackList.remove(i);
                }
            }
            switch (hideVersion) {
                case 1 :
                    Shell.su("/magisk/.core/magiskhide/rm " + appToCheck);
                    break;
                case 2 :
                    Shell.su("/su/suhide/rm " + UID);
                    break;
                case 3 :
                    Shell.su("/su/suhide/rm " + UID + "&& /magisk/.core/magiskhide/rm " + appToCheck);
                    break;
                default :
                    break;

            }

        }
        Logger.dev("Committing set, value is: " + arrayBlackList.toString());
        SharedPreferences.Editor editor = prefs.edit();
        editor.putStringSet("auto_blacklist", new HashSet<>(arrayBlackList));
        editor.apply();
        listadaptor.UpdateRootStatusView(position, v);

    }



    private List<ApplicationInfo> checkForLaunchIntent(List<ApplicationInfo> list) {
        ArrayList<ApplicationInfo> applist = new ArrayList<>();
        for (ApplicationInfo info : list) {
            try {
                if (null != packageManager.getLaunchIntentForPackage(info.packageName)) {
                    if (!info.packageName.contains("magisk")) {
                        applist.add(info);
                    }
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
            return o1.loadLabel(packageManager).toString().compareToIgnoreCase(o2.loadLabel(packageManager).toString());
        }
    }

    private class LoadApplications extends AsyncTask<Void, Void, Void> {
        private ProgressDialog progress = null;

        @Override
        protected Void doInBackground(Void... params) {
            applist = checkForLaunchIntent(packageManager.getInstalledApplications(PackageManager.GET_META_DATA));
            listadaptor = new ApplicationAdapter(getActivity(),
                    R.layout.list_item_app, applist);

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