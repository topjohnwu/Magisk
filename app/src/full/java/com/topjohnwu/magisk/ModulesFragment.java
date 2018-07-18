package com.topjohnwu.magisk;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
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
import android.widget.TextView;

import com.topjohnwu.magisk.adapters.ModulesAdapter;
import com.topjohnwu.magisk.asyncs.LoadModules;
import com.topjohnwu.magisk.components.Fragment;
import com.topjohnwu.magisk.container.Module;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.superuser.Shell;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class ModulesFragment extends Fragment implements Topic.Subscriber {

    private Unbinder unbinder;
    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    @BindView(R.id.empty_rv) TextView emptyRv;
    @OnClick(R.id.fab)
    public void selectFile() {
        runWithPermission(new String[] { Manifest.permission.WRITE_EXTERNAL_STORAGE }, () -> {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.setType("application/zip");
            startActivityForResult(intent, Const.ID.FETCH_ZIP);
        });
    }

    private List<Module> listModules = new ArrayList<>();

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_modules, container, false);
        unbinder = ButterKnife.bind(this, view);
        setHasOptionsMenu(true);

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            recyclerView.setVisibility(View.GONE);
            new LoadModules().exec();
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

        getActivity().setTitle(R.string.modules);

        return view;
    }

    @Override
    public void onTopicPublished(Topic topic) {
        updateUI();
    }

    @Override
    public Topic[] getSubscription() {
        return new Topic[] { getApplication().moduleLoadDone };
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == Const.ID.FETCH_ZIP && resultCode == Activity.RESULT_OK && data != null) {
            // Get the URI of the selected file
            Intent intent = new Intent(getActivity(), FlashActivity.class);
            intent.setData(data.getData()).putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_ZIP);
            startActivity(intent);
        }
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.menu_reboot, menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.reboot:
                Shell.Async.su("/system/bin/reboot");
                return true;
            case R.id.reboot_recovery:
                Shell.Async.su("/system/bin/reboot recovery");
                return true;
            case R.id.reboot_bootloader:
                Shell.Async.su("/system/bin/reboot bootloader");
                return true;
            case R.id.reboot_download:
                Shell.Async.su("/system/bin/reboot download");
                return true;
            default:
                return false;
        }
    }

    private void updateUI() {
        listModules.clear();
        listModules.addAll(getApplication().moduleMap.values());
        if (listModules.size() == 0) {
            emptyRv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
        } else {
            emptyRv.setVisibility(View.GONE);
            recyclerView.setVisibility(View.VISIBLE);
            recyclerView.setAdapter(new ModulesAdapter(listModules));
        }
        mSwipeRefreshLayout.setRefreshing(false);
    }
}
