package com.topjohnwu.magisk;

import android.app.Fragment;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Shell;

import java.util.Collections;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class MagiskHideFragment extends Fragment {

    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;

    private PackageManager packageManager;
    private View mView;
    private ApplicationAdapter appAdapter;

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

        new LoadApps().exec();
        return mView;
    }

    @Override
    public void onResume() {
        super.onResume();
        mView = this.getView();
    }

    private class LoadApps extends Async.RootTask<Void, Void, Void> {
        @Override
        protected Void doInBackground(Void... voids) {
            List<ApplicationInfo> listApps = packageManager.getInstalledApplications(PackageManager.GET_META_DATA);
            Collections.sort(listApps, (a, b) -> a.loadLabel(packageManager).toString().toLowerCase()
                    .compareTo(b.loadLabel(packageManager).toString().toLowerCase()));
            List<String> hideList = Shell.su(Async.MAGISK_HIDE_PATH + "list");
            appAdapter = new ApplicationAdapter(listApps, hideList);
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            updateUI();
        }
    }

    private void updateUI() {
        recyclerView.setAdapter(appAdapter);
        recyclerView.setVisibility(View.VISIBLE);
        mSwipeRefreshLayout.setRefreshing(false);
    }

}