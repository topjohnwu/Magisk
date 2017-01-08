package com.topjohnwu.magisk;

import android.app.Fragment;
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
import com.topjohnwu.magisk.utils.CallbackHandler;
import com.topjohnwu.magisk.utils.Logger;

import java.util.Arrays;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class MagiskHideFragment extends Fragment implements CallbackHandler.EventListener {

    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;

    // Don't show in list...
    public static final List<String> BLACKLIST =  Arrays.asList(
            "android",
            "com.topjohnwu.magisk",
            "com.google.android.gms",
            "com.google.android.apps.walletnfcrel",
            "com.nianticlabs.pokemongo"
    );
    public static CallbackHandler.Event packageLoadDone = new CallbackHandler.Event();

    private PackageManager packageManager;
    private ApplicationAdapter appAdapter = new ApplicationAdapter();

    private SearchView.OnQueryTextListener searchListener;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.magisk_hide_fragment, container, false);
        ButterKnife.bind(this, view);

        packageManager = getActivity().getPackageManager();

        mSwipeRefreshLayout.setRefreshing(true);
        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            recyclerView.setVisibility(View.GONE);
            new Async.LoadApps(packageManager).exec();
        });

        recyclerView.setAdapter(appAdapter);

        searchListener = new SearchView.OnQueryTextListener() {
            @Override
            public boolean onQueryTextSubmit(String query) {
                appAdapter.filter(query);
                return false;
            }

            @Override
            public boolean onQueryTextChange(String newText) {
                appAdapter.filter(newText);
                return false;
            }
        };

        return view;
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
        getActivity().setTitle(R.string.magiskhide);
        CallbackHandler.register(packageLoadDone, this);
        if (packageLoadDone.isTriggered) {
            onTrigger(packageLoadDone);
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        CallbackHandler.unRegister(packageLoadDone, this);
    }

    @Override
    public void onTrigger(CallbackHandler.Event event) {
        Logger.dev("MagiskHideFragment: UI refresh");
        Async.LoadApps.Result result = (Async.LoadApps.Result) event.getResult();
        appAdapter.setLists(result.listApps, result.hideList);
        recyclerView.setVisibility(View.VISIBLE);
        mSwipeRefreshLayout.setRefreshing(false);
    }
}
