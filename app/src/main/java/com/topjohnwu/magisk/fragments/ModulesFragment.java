package com.topjohnwu.magisk.fragments;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.recyclerview.widget.RecyclerView;
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout;

import com.topjohnwu.magisk.ClassMap;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.FlashActivity;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.adapters.ModulesAdapter;
import com.topjohnwu.magisk.components.BaseFragment;
import com.topjohnwu.magisk.container.Module;
import com.topjohnwu.magisk.utils.Event;
import com.topjohnwu.magisk.utils.RootUtils;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import butterknife.BindView;
import butterknife.OnClick;

public class ModulesFragment extends BaseFragment {

    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    @BindView(R.id.empty_rv) TextView emptyRv;

    @OnClick(R.id.fab)
    void selectFile() {
        runWithPermission(new String[] { Manifest.permission.WRITE_EXTERNAL_STORAGE }, () -> {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.setType("application/zip");
            startActivityForResult(intent, Const.ID.FETCH_ZIP);
        });
    }

    private List<Module> listModules = new ArrayList<>();

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_modules, container, false);
        unbinder = new ModulesFragment_ViewBinding(this, view);
        setHasOptionsMenu(true);

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            recyclerView.setVisibility(View.GONE);
            Utils.loadModules();
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

        requireActivity().setTitle(R.string.modules);

        return view;
    }

    @Override
    public int[] getListeningEvents() {
        return new int[] {Event.MODULE_LOAD_DONE};
    }

    @Override
    public void onEvent(int event) {
        updateUI(Event.getResult(event));
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == Const.ID.FETCH_ZIP && resultCode == Activity.RESULT_OK && data != null) {
            // Get the URI of the selected file
            Intent intent = new Intent(getActivity(), ClassMap.get(FlashActivity.class));
            intent.setData(data.getData()).putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_ZIP);
            startActivity(intent);
        }
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.menu_reboot, menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.reboot:
                RootUtils.reboot();
                return true;
            case R.id.reboot_recovery:
                Shell.su("/system/bin/reboot recovery").submit();
                return true;
            case R.id.reboot_bootloader:
                Shell.su("/system/bin/reboot bootloader").submit();
                return true;
            case R.id.reboot_download:
                Shell.su("/system/bin/reboot download").submit();
                return true;
            default:
                return false;
        }
    }

    private void updateUI(Map<String, Module> moduleMap) {
        listModules.clear();
        listModules.addAll(moduleMap.values());
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
