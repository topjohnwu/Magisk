package com.topjohnwu.magisk.fragments;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SearchView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.adapters.ApplicationAdapter;
import com.topjohnwu.magisk.components.BaseFragment;
import com.topjohnwu.magisk.utils.Topic;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.recyclerview.widget.RecyclerView;
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout;
import butterknife.BindView;

public class MagiskHideFragment extends BaseFragment implements Topic.Subscriber {

    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    SearchView search;

    private ApplicationAdapter appAdapter;

    private SearchView.OnQueryTextListener searchListener;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(true);
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_magisk_hide, container, false);
        unbinder = new MagiskHideFragment_ViewBinding(this, view);

        appAdapter = new ApplicationAdapter(requireActivity());
        recyclerView.setAdapter(appAdapter);

        mSwipeRefreshLayout.setRefreshing(true);
        mSwipeRefreshLayout.setOnRefreshListener(appAdapter::refresh);

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

        requireActivity().setTitle(R.string.magiskhide);

        return view;
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.menu_magiskhide, menu);
        search = (SearchView) menu.findItem(R.id.app_search).getActionView();
        search.setOnQueryTextListener(searchListener);
    }

    @Override
    public int[] getSubscribedTopics() {
        return new int[] {Topic.MAGISK_HIDE_DONE};
    }

    @Override
    public void onPublish(int topic, Object[] result) {
        mSwipeRefreshLayout.setRefreshing(false);
        appAdapter.filter(search.getQuery().toString());
    }
}
