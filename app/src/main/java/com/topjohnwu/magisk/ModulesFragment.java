package com.topjohnwu.magisk;

import android.app.Fragment;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.design.widget.FloatingActionButton;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.ipaulpro.afilechooser.utils.FileUtils;
import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.ModuleHelper;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesFragment extends Fragment {
    private static final int FETCH_ZIP_CODE = 2;

    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    @BindView(R.id.empty_rv) TextView emptyTv;
    @BindView(R.id.fab) FloatingActionButton fabio;

    private SharedPreferences prefs;
    private List<Module> listModules = new ArrayList<>();
    private View mView;
    private SharedPreferences.OnSharedPreferenceChangeListener listener;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        mView = inflater.inflate(R.layout.modules_fragment, container, false);
        ButterKnife.bind(this, mView);

        fabio.setOnClickListener(v -> {
            Intent getContentIntent = FileUtils.createGetContentIntent(null);
            getContentIntent.setType("application/zip");
            Intent fileIntent = Intent.createChooser(getContentIntent, "Select a file");
            startActivityForResult(fileIntent, FETCH_ZIP_CODE);
        });

        prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            recyclerView.setVisibility(View.GONE);
            prefs.edit().putBoolean("module_done", false).apply();
            new Async.LoadModules(getActivity()).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
        });

        if (prefs.getBoolean("module_done", false)) {
            updateUI();
        }

        listener = (pref, s) -> {
            if (s.equals("module_done")) {
                if (pref.getBoolean(s, false)) {
                    Logger.dev("ModulesFragment: UI refresh triggered");
                    updateUI();
                }
            }
        };

        return mView;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (data != null) {
            // Get the URI of the selected file
            final Uri uri = data.getData();
            // Get the file path from the URI
            new Async.FlashZIP(getActivity(), uri).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
        }

    }

    @Override
    public void onResume() {
        super.onResume();
        mView = this.getView();
        prefs.registerOnSharedPreferenceChangeListener(listener);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        prefs.unregisterOnSharedPreferenceChangeListener(listener);
    }

    private void updateUI() {
        ModuleHelper.getModuleList(listModules);
        if (listModules.size() == 0) {
            emptyTv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
        } else {
            recyclerView.setVisibility(View.VISIBLE);
            recyclerView.setAdapter(new ModulesAdapter(listModules));
        }
        mSwipeRefreshLayout.setRefreshing(false);
    }

}
