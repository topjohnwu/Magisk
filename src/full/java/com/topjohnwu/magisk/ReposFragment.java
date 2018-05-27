package com.topjohnwu.magisk;

import android.app.AlertDialog;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SearchView;
import android.widget.TextView;

import com.topjohnwu.magisk.adapters.ReposAdapter;
import com.topjohnwu.magisk.asyncs.UpdateRepos;
import com.topjohnwu.magisk.components.Fragment;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Topic;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class ReposFragment extends Fragment implements Topic.Subscriber {

    private Unbinder unbinder;
    private MagiskManager mm;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    @BindView(R.id.empty_rv) TextView emptyRv;
    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;

    public static ReposAdapter adapter;

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
        mm = getApplication();

        mSwipeRefreshLayout.setRefreshing(mm.repoLoadDone.isPending());

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            recyclerView.setVisibility(View.VISIBLE);
            emptyRv.setVisibility(View.GONE);
            new UpdateRepos(true).exec();
        });

        getActivity().setTitle(R.string.downloads);

        return view;
    }

    @Override
    public void onResume() {
        adapter = new ReposAdapter(mm.repoDB, mm.moduleMap);
        recyclerView.setAdapter(adapter);
        super.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
        adapter = null;
    }

    @Override
    public void onTopicPublished(Topic topic) {
        mSwipeRefreshLayout.setRefreshing(false);
        recyclerView.setVisibility(adapter.getItemCount() == 0 ? View.GONE : View.VISIBLE);
        emptyRv.setVisibility(adapter.getItemCount() == 0 ? View.VISIBLE : View.GONE);
    }

    @Override
    public Topic[] getSubscription() {
        return new Topic[] { mm.repoLoadDone };
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
                adapter.filter(newText);
                return false;
            }
        });
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == R.id.repo_sort) {
            new AlertDialog.Builder(getActivity())
                .setTitle(R.string.sorting_order)
                .setSingleChoiceItems(R.array.sorting_orders, mm.repoOrder, (d, which) -> {
                    mm.repoOrder = which;
                    mm.prefs.edit().putInt(Const.Key.REPO_ORDER, mm.repoOrder).apply();
                    adapter.notifyDBChanged();
                    d.dismiss();
                }).show();
        }
        return true;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }
}
