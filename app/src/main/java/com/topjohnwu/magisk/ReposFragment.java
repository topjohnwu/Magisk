package com.topjohnwu.magisk;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SearchView;
import android.widget.TextView;

import com.topjohnwu.magisk.adapters.ReposAdapter;
import com.topjohnwu.magisk.asyncs.LoadRepos;
import com.topjohnwu.magisk.components.Fragment;
import com.topjohnwu.magisk.utils.CallbackEvent;
import com.topjohnwu.magisk.utils.Logger;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class ReposFragment extends Fragment implements CallbackEvent.Listener<Void> {

    private Unbinder unbinder;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    @BindView(R.id.empty_rv) TextView emptyRv;
    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;

    private ReposAdapter adapter;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(true);
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_repos, container, false);
        unbinder = ButterKnife.bind(this, view);

        adapter = new ReposAdapter(getApplication().repoMap);

        recyclerView.setAdapter(adapter);

        mSwipeRefreshLayout.setRefreshing(true);

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            recyclerView.setVisibility(View.GONE);
            new LoadRepos(getActivity()).exec();
        });

        if (getApplication().repoLoadDone.isTriggered) {
            onTrigger(null);
        }

        return view;
    }

    @Override
    public void onTrigger(CallbackEvent<Void> event) {
        Logger.dev("ReposFragment: UI refresh triggered");
        if (getApplication().repoMap.isEmpty()) {
            recyclerView.setVisibility(View.GONE);
            emptyRv.setVisibility(View.VISIBLE);
        } else {
            adapter.filter(getApplication().moduleMap, "");
            recyclerView.setVisibility(View.VISIBLE);
            emptyRv.setVisibility(View.GONE);
            mSwipeRefreshLayout.setRefreshing(false);
        }
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.menu_repo, menu);
        SearchView search = (SearchView) menu.findItem(R.id.repo_search).getActionView();
        search.setOnQueryTextListener(new SearchView.OnQueryTextListener() {
            @Override
            public boolean onQueryTextSubmit(String query) {
                return false;
            }

            @Override
            public boolean onQueryTextChange(String newText) {
                adapter.filter(getApplication().moduleMap, newText);
                return false;
            }
        });
    }

    @Override
    public void onStart() {
        super.onStart();
        getApplication().repoLoadDone.register(this);
        getActivity().setTitle(R.string.downloads);
    }

    @Override
    public void onStop() {
        getApplication().repoLoadDone.unRegister(this);
        super.onStop();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }
}
