package com.topjohnwu.magisk;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v7.widget.CardView;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Repo;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public abstract class BaseRepoFragment extends Fragment {

    @BindView(R.id.recyclerView)
    RecyclerView recyclerView;
    @BindView(R.id.empty_rv)
    TextView emptyTv;



    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.single_module_fragment, container, false);


        ButterKnife.bind(this, view);

        if (listRepos().size() == 0) {
            emptyTv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);

            return view;
        }

        recyclerView.setAdapter(new ReposAdapter(listRepos()) {

        });
        return view;
    }


    protected abstract List<Repo> listRepos();
}
