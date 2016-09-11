package com.topjohnwu.magisk;

import android.content.Context;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.module.RepoHelper;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ReposFragment extends Fragment {

    public static List<Repo> mListRepos = new ArrayList<>();
    @BindView(R.id.recyclerView)
    RecyclerView recyclerView;
    @BindView(R.id.empty_rv)
    TextView emptyTv;
    @BindView(R.id.swipeRefreshLayout)
    SwipeRefreshLayout swipeRefreshLayout;
    private RepoHelper.TaskDelegate taskDelegate;

    @Nullable

    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.single_repo_fragment, container, false);
        ButterKnife.bind(this, view);
        swipeRefreshLayout.setOnRefreshListener(() -> {
            Log.d("Magisk","ReposFragment: WTF IM CALLED");
            this.LoadRepo(true);
        });
        LoadRepo(false);
        setHasOptionsMenu(false);

        if (mListRepos.size() == 0) {
            emptyTv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
            return view;
        }
        Log.d("Magisk", "ReposFragment: ListRepos size is " + listRepos().size());
        recyclerView.setAdapter(new ReposAdapter(this, mListRepos));
        return view;
    }

    private void LoadRepo (boolean doReload) {
        taskDelegate = result -> {
            if (result.equals("Complete")) {
                Log.d("Magisk", "ReposFragment, got delegate");
                UpdateUI();
            }


        };
        Log.d("Magisk","ReposFragment, LoadRepo called");
        mListRepos.clear();
        RepoHelper mr = new RepoHelper();
        List<Repo> magiskRepos = mr.listRepos(getActivity(), doReload,taskDelegate);

        for (Repo repo : magiskRepos) {
            Log.d("Magisk", "ReposFragment: Adding repo from string " + repo.getId());
            mListRepos.add(repo);
        }

    }



    @Override
    public void onResume() {
        super.onResume();
        LoadRepo(false);

    }



    protected List<Repo> listRepos() {
        return mListRepos;
    }

    private void UpdateUI() {
        Log.d("Magisk","ReposFragment: UpdateUI Called, size is " + listRepos().size());

        if (listRepos().size() == 0) {
            emptyTv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);

        }
        Log.d("Magisk", "ReposFragment: ListRepos size is " + listRepos().size());
        recyclerView.setAdapter(new ReposAdapter(this, listRepos()));
        if (swipeRefreshLayout.isRefreshing()) {
            swipeRefreshLayout.setRefreshing(false);
        }


    }







}
