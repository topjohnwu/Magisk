package com.topjohnwu.magisk;

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
import android.widget.TextView;

import com.topjohnwu.magisk.adapters.ReposAdapter;
import com.topjohnwu.magisk.adapters.SimpleSectionedRecyclerViewAdapter;
import com.topjohnwu.magisk.asyncs.LoadRepos;
import com.topjohnwu.magisk.asyncs.ParallelTask;
import com.topjohnwu.magisk.components.Fragment;
import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.CallbackEvent;
import com.topjohnwu.magisk.utils.Logger;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class ReposFragment extends Fragment implements CallbackEvent.Listener<Void> {

    private Unbinder unbinder;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    @BindView(R.id.empty_rv) TextView emptyRv;
    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;

    private List<Repo> mUpdateRepos = new ArrayList<>();
    private List<Repo> mInstalledRepos = new ArrayList<>();
    private List<Repo> mOthersRepos = new ArrayList<>();
    private List<Repo> fUpdateRepos = new ArrayList<>();
    private List<Repo> fInstalledRepos = new ArrayList<>();
    private List<Repo> fOthersRepos = new ArrayList<>();

    private SimpleSectionedRecyclerViewAdapter mSectionedAdapter;

    private SearchView.OnQueryTextListener searchListener;

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

        mSectionedAdapter = new SimpleSectionedRecyclerViewAdapter(R.layout.section,
                R.id.section_text, new ReposAdapter(fUpdateRepos, fInstalledRepos, fOthersRepos));

        recyclerView.setAdapter(mSectionedAdapter);

        mSwipeRefreshLayout.setRefreshing(true);

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            recyclerView.setVisibility(View.GONE);
            new LoadRepos(getActivity()).exec();
        });

        if (getApplication().repoLoadDone.isTriggered) {
            reloadRepos();
            updateUI();
        }

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

        return view;
    }

    @Override
    public void onTrigger(CallbackEvent<Void> event) {
        Logger.dev("ReposFragment: UI refresh triggered");
        reloadRepos();
        updateUI();
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.menu_repo, menu);
        SearchView search = (SearchView) MenuItemCompat.getActionView(menu.findItem(R.id.repo_search));
        search.setOnQueryTextListener(searchListener);
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

    private void reloadRepos() {
        mUpdateRepos.clear();
        mInstalledRepos.clear();
        mOthersRepos.clear();
        for (Repo repo : getApplication().repoMap.values()) {
            Module module = getApplication().moduleMap.get(repo.getId());
            if (module != null) {
                if (repo.getVersionCode() > module.getVersionCode()) {
                    mUpdateRepos.add(repo);
                } else {
                    mInstalledRepos.add(repo);
                }
            } else {
                mOthersRepos.add(repo);
            }
        }
        fUpdateRepos.clear();
        fInstalledRepos.clear();
        fOthersRepos.clear();
        fUpdateRepos.addAll(mUpdateRepos);
        fInstalledRepos.addAll(mInstalledRepos);
        fOthersRepos.addAll(mOthersRepos);
    }

    private void updateUI() {
        if (fUpdateRepos.size() + fInstalledRepos.size() + fOthersRepos.size() == 0) {
            emptyRv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
        } else {
            List<SimpleSectionedRecyclerViewAdapter.Section> sections = new ArrayList<>();
            if (!fUpdateRepos.isEmpty()) {
                sections.add(new SimpleSectionedRecyclerViewAdapter.Section(0, getString(R.string.update_available)));
            }
            if (!fInstalledRepos.isEmpty()) {
                sections.add(new SimpleSectionedRecyclerViewAdapter.Section(fUpdateRepos.size(), getString(R.string.installed)));
            }
            if (!fOthersRepos.isEmpty()) {
                sections.add(new SimpleSectionedRecyclerViewAdapter.Section(fUpdateRepos.size() + fInstalledRepos.size(), getString(R.string.not_installed)));
            }
            SimpleSectionedRecyclerViewAdapter.Section[] array = sections.toArray(new SimpleSectionedRecyclerViewAdapter.Section[sections.size()]);
            mSectionedAdapter.setSections(array);
            emptyRv.setVisibility(View.GONE);
            recyclerView.setVisibility(View.VISIBLE);
        }
        mSwipeRefreshLayout.setRefreshing(false);
    }

    private class FilterApps extends ParallelTask<String, Void, Void> {
        @Override
        protected Void doInBackground(String... strings) {
            String newText = strings[0];
            fUpdateRepos.clear();
            fInstalledRepos.clear();
            fOthersRepos.clear();
            for (Repo repo: mUpdateRepos) {
                if (repo.getName().toLowerCase().contains(newText.toLowerCase())
                        || repo.getAuthor().toLowerCase().contains(newText.toLowerCase())
                        || repo.getDescription().toLowerCase().contains(newText.toLowerCase())
                        ) {
                    fUpdateRepos.add(repo);
                }
            }
            for (Repo repo: mInstalledRepos) {
                if (repo.getName().toLowerCase().contains(newText.toLowerCase())
                        || repo.getAuthor().toLowerCase().contains(newText.toLowerCase())
                        || repo.getDescription().toLowerCase().contains(newText.toLowerCase())
                        ) {
                    fInstalledRepos.add(repo);
                }
            }
            for (Repo repo: mOthersRepos) {
                if (repo.getName().toLowerCase().contains(newText.toLowerCase())
                        || repo.getAuthor().toLowerCase().contains(newText.toLowerCase())
                        || repo.getDescription().toLowerCase().contains(newText.toLowerCase())
                        ) {
                    fOthersRepos.add(repo);
                }
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            updateUI();
        }
    }

}
