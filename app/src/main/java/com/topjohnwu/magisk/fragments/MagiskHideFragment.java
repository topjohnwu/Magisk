package com.topjohnwu.magisk.fragments;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SearchView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.recyclerview.widget.RecyclerView;
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout;

import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.adapters.ApplicationAdapter;
import com.topjohnwu.magisk.components.BaseFragment;
import com.topjohnwu.magisk.utils.Topic;

import butterknife.BindView;
import me.drakeet.multitype.MultiTypeAdapter;

public class MagiskHideFragment extends BaseFragment implements Topic.Subscriber {

    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;

    private SearchView search;
    private ApplicationAdapter applicationAdapter;
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

        MultiTypeAdapter adapter = new MultiTypeAdapter();
        applicationAdapter = new ApplicationAdapter(requireActivity(), adapter);
        recyclerView.setAdapter(adapter);

        mSwipeRefreshLayout.setRefreshing(true);
        mSwipeRefreshLayout.setOnRefreshListener(applicationAdapter::refresh);

        searchListener = new SearchView.OnQueryTextListener() {
            @Override
            public boolean onQueryTextSubmit(String query) {
                applicationAdapter.filter(query);
                return false;
            }

            @Override
            public boolean onQueryTextChange(String newText) {
                applicationAdapter.filter(newText);
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
        menu.findItem(R.id.show_system).setChecked(Config.get(Config.Key.SHOW_SYSTEM_APP));
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == R.id.show_system) {
            boolean showSystem = !item.isChecked();
            item.setChecked(showSystem);
            Config.set(Config.Key.SHOW_SYSTEM_APP, showSystem);
            applicationAdapter.setShowSystem(showSystem);
            applicationAdapter.filter(search.getQuery().toString());
        }
        return true;
    }

    @Override
    public int[] getSubscribedTopics() {
        return new int[] {Topic.MAGISK_HIDE_DONE};
    }

    @Override
    public void onPublish(int topic, Object[] result) {
        mSwipeRefreshLayout.setRefreshing(false);
        applicationAdapter.filter(search.getQuery().toString());
    }
}
