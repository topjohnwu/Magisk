package com.topjohnwu.magisk;

import android.app.Activity;
import android.app.Fragment;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.github.clans.fab.FloatingActionButton;
import com.topjohnwu.magisk.adapters.ModulesAdapter;
import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.CallbackHandler;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.ModuleHelper;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesFragment extends Fragment implements CallbackHandler.EventListener {

    public static final CallbackHandler.Event moduleLoadDone = new CallbackHandler.Event();

    private static final int FETCH_ZIP_CODE = 2;

    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    @BindView(R.id.empty_rv) TextView emptyTv;
    @BindView(R.id.fab) FloatingActionButton fabio;

    private List<Module> listModules = new ArrayList<>();

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.modules_fragment, container, false);
        ButterKnife.bind(this, view);

        fabio.setOnClickListener(v -> {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.setType("application/zip");
            startActivityForResult(intent, FETCH_ZIP_CODE);
        });

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            recyclerView.setVisibility(View.GONE);
            new Async.LoadModules().exec();
        });

        recyclerView.addOnScrollListener(new RecyclerView.OnScrollListener() {
            @Override
            public void onScrolled(RecyclerView recyclerView, int dx, int dy) {
                mSwipeRefreshLayout.setEnabled(recyclerView.getChildAt(0).getTop() >= 0);
            }

            @Override
            public void onScrollStateChanged(RecyclerView recyclerView, int newState) {
                super.onScrollStateChanged(recyclerView, newState);
            }
        });

        if (moduleLoadDone.isTriggered) {
            updateUI();
        }

        return view;
    }

    @Override
    public void onTrigger(CallbackHandler.Event event) {
        Logger.dev("ModulesFragment: UI refresh triggered");
        updateUI();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == FETCH_ZIP_CODE && resultCode == Activity.RESULT_OK && data != null) {
            // Get the URI of the selected file
            final Uri uri = data.getData();
            new Async.FlashZIP(getActivity(), uri).exec();
        }

    }

    @Override
    public void onStart() {
        super.onStart();
        CallbackHandler.register(moduleLoadDone, this);
        getActivity().setTitle(R.string.modules);
    }

    @Override
    public void onStop() {
        CallbackHandler.unRegister(moduleLoadDone, this);
        super.onStop();
    }

    private void updateUI() {
        ModuleHelper.getModuleList(listModules);
        if (listModules.size() == 0) {
            emptyTv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
        } else {
            emptyTv.setVisibility(View.GONE);
            recyclerView.setVisibility(View.VISIBLE);
            recyclerView.setAdapter(new ModulesAdapter(listModules));
        }
        mSwipeRefreshLayout.setRefreshing(false);
    }
}
