package com.topjohnwu.magisk;

import android.app.Fragment;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.view.MenuItemCompat;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SearchView;

import com.topjohnwu.magisk.adapters.ApplicationAdapter;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Shell;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class MagiskHideFragment extends Fragment {

    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;

    private PackageManager packageManager;
    private View mView;
    private List<ApplicationInfo> listApps = new ArrayList<>(), fListApps = new ArrayList<>();
    private List<String> hideList = new ArrayList<>();
    private ApplicationAdapter appAdapter = new ApplicationAdapter(fListApps, hideList);

    private SearchView.OnQueryTextListener searchListener;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        mView = inflater.inflate(R.layout.magisk_hide_fragment, container, false);
        ButterKnife.bind(this, mView);

        packageManager = getActivity().getPackageManager();

        mSwipeRefreshLayout.setRefreshing(true);
        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            recyclerView.setVisibility(View.GONE);
            new LoadApps().exec();
        });

        recyclerView.setAdapter(appAdapter);

        searchListener = new SearchView.OnQueryTextListener() {
            @Override
            public boolean onQueryTextSubmit(String query) {
                return false;
            }

            @Override
            public boolean onQueryTextChange(String newText) {
                new FilterApps().exec(newText);
                return false;
            }
        };

        new LoadApps().exec();
        return mView;
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.menu_magiskhide, menu);
        SearchView search = (SearchView) MenuItemCompat.getActionView(menu.findItem(R.id.app_search));
        search.setOnQueryTextListener(searchListener);
    }

    @Override
    public void onResume() {
        super.onResume();
        setHasOptionsMenu(true);
        mView = this.getView();
        getActivity().setTitle(R.string.magiskhide);
    }

    private class LoadApps extends Async.RootTask<Void, Void, Void> {
        @Override
        protected Void doInBackground(Void... voids) {
            listApps.clear();
            hideList.clear();
            fListApps.clear();
            listApps.addAll(packageManager.getInstalledApplications(PackageManager.GET_META_DATA));
            Collections.sort(listApps, (a, b) -> a.loadLabel(packageManager).toString().toLowerCase()
                    .compareTo(b.loadLabel(packageManager).toString().toLowerCase()));
            hideList.addAll(Shell.su(Async.MAGISK_HIDE_PATH + "list"));
            fListApps.addAll(listApps);
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            updateUI();
        }
    }

    private class FilterApps extends Async.NormalTask<String, Void, Void> {
        @Override
        protected Void doInBackground(String... strings) {
            String newText = strings[0];
            fListApps.clear();
            for (ApplicationInfo info : listApps) {
                if (info.loadLabel(packageManager).toString().toLowerCase().contains(newText.toLowerCase())
                        || info.packageName.toLowerCase().contains(newText.toLowerCase())) {
                    fListApps.add(info);
                }
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            appAdapter.notifyDataSetChanged();
        }
    }

    private void updateUI() {
        appAdapter.notifyDataSetChanged();
        recyclerView.setVisibility(View.VISIBLE);
        mSwipeRefreshLayout.setRefreshing(false);
    }

}