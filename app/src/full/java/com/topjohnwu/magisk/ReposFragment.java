package com.topjohnwu.magisk;

import android.app.AlertDialog;
import android.os.Bundle;
import android.support.annotation.NonNull;
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
import com.topjohnwu.magisk.components.BaseFragment;
import com.topjohnwu.magisk.container.Module;
import com.topjohnwu.magisk.utils.Topic;

import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class ReposFragment extends BaseFragment implements Topic.Subscriber {

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
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_repos, container, false);
        unbinder = ButterKnife.bind(this, view);

        mSwipeRefreshLayout.setRefreshing(true);
        recyclerView.setVisibility(View.GONE);

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            recyclerView.setVisibility(View.VISIBLE);
            emptyRv.setVisibility(View.GONE);
            new UpdateRepos().exec(true);
        });

        requireActivity().setTitle(R.string.downloads);

        return view;
    }

    @Override
    public int[] getSubscribedTopics() {
        return new int[] {Topic.MODULE_LOAD_DONE, Topic.REPO_LOAD_DONE};
    }

    @Override
    public void onPublish(int topic, Object[] result) {
        if (topic == Topic.MODULE_LOAD_DONE) {
            adapter = new ReposAdapter(mm.repoDB, (Map<String, Module>) result[0]);
            mm.repoDB.registerAdapter(adapter);
            recyclerView.setAdapter(adapter);
            recyclerView.setVisibility(View.VISIBLE);
            emptyRv.setVisibility(View.GONE);
        }
        if (Topic.isPublished(getSubscribedTopics())) {
            mSwipeRefreshLayout.setRefreshing(false);
            recyclerView.setVisibility(adapter.getItemCount() == 0 ? View.GONE : View.VISIBLE);
            emptyRv.setVisibility(adapter.getItemCount() == 0 ? View.VISIBLE : View.GONE);
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
                .setSingleChoiceItems(R.array.sorting_orders, Data.repoOrder, (d, which) -> {
                    Data.repoOrder = which;
                    mm.prefs.edit().putInt(Const.Key.REPO_ORDER, Data.repoOrder).apply();
                    adapter.notifyDBChanged();
                    d.dismiss();
                }).show();
        }
        return true;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        mm.repoDB.unregisterAdapter();
        unbinder.unbind();
    }
}
