package com.topjohnwu.magisk;

import android.app.Fragment;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.ModuleHelper;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ReposFragment extends Fragment {

    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    @BindView(R.id.empty_rv) TextView emptyTv;
    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;
    private List<Repo> mListRepos = new ArrayList<>();
    private List<Repo> mUpdateRepos = new ArrayList<>();
    private List<Repo> mInstalledRepos = new ArrayList<>();
    private List<Repo> mOthersRepos = new ArrayList<>();
    private SharedPreferences.OnSharedPreferenceChangeListener listener;
    private SharedPreferences prefs;

    @Nullable

    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.repos_fragment, container, false);
        ButterKnife.bind(this, view);

        mSwipeRefreshLayout.setRefreshing(true);

        prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            recyclerView.setVisibility(View.GONE);
            prefs.edit().putBoolean("repo_done", false).apply();
            new Async.LoadRepos(getActivity()).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        });

        if (prefs.getBoolean("repo_done", false)) {
            updateUI();
        }

        listener = (pref, s) -> {
            if (s.equals("repo_done")) {
                if (pref.getBoolean(s, false)) {
                    Logger.dev("ReposFragment: UI refresh triggered");
                    updateUI();
                }
            }
        };

        return view;
    }

    @Override
    public void onAttachFragment(Fragment childFragment) {
        super.onAttachFragment(childFragment);
    }

    @Override
    public void onResume() {
        super.onResume();
        prefs.registerOnSharedPreferenceChangeListener(listener);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        prefs.unregisterOnSharedPreferenceChangeListener(listener);
    }

    private void updateUI() {
        ModuleHelper.getRepoLists(mUpdateRepos, mInstalledRepos, mOthersRepos);
        mListRepos.clear();
        mListRepos.addAll(mUpdateRepos);
        mListRepos.addAll(mInstalledRepos);
        mListRepos.addAll(mOthersRepos);
        if (mListRepos.size() == 0) {
            emptyTv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
        } else {
            List<SimpleSectionedRecyclerViewAdapter.Section> sections = new ArrayList<>();
            if (!mUpdateRepos.isEmpty()) {
                sections.add(new SimpleSectionedRecyclerViewAdapter.Section(0, getString(R.string.update_available)));
            }
            if (!mInstalledRepos.isEmpty()) {
                sections.add(new SimpleSectionedRecyclerViewAdapter.Section(mUpdateRepos.size(), getString(R.string.installed)));
            }
            if (!mOthersRepos.isEmpty()) {
                sections.add(new SimpleSectionedRecyclerViewAdapter.Section(mUpdateRepos.size() + mInstalledRepos.size(), getString(R.string.not_installed)));
            }
            SimpleSectionedRecyclerViewAdapter.Section[] array = sections.toArray(new SimpleSectionedRecyclerViewAdapter.Section[sections.size()]);
            SimpleSectionedRecyclerViewAdapter mSectionedAdapter = new
                    SimpleSectionedRecyclerViewAdapter(getActivity(), R.layout.section, R.id.section_text, new ReposAdapter(mListRepos));
            mSectionedAdapter.setSections(array);
            recyclerView.setVisibility(View.VISIBLE);
            recyclerView.setAdapter(mSectionedAdapter);
        }
        mSwipeRefreshLayout.setRefreshing(false);
    }

}
