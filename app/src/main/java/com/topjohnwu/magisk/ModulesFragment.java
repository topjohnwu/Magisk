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
import android.support.design.widget.Snackbar;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.TextView;

import com.ipaulpro.afilechooser.FileInfo;
import com.ipaulpro.afilechooser.utils.FileUtils;
import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.utils.Async;

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
    public static List<Module> listModules = new ArrayList<>();

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View viewMain = inflater.inflate(R.layout.modules_fragment, container, false);
        ButterKnife.bind(this, viewMain);

        mSwipeRefreshLayout.setRefreshing(true);
        fabio.setOnClickListener(v -> {
            Intent getContentIntent = FileUtils.createGetContentIntent(null);
            getContentIntent.setType("application/zip");
            Intent fileIntent = Intent.createChooser(getContentIntent, "Select a file");
            startActivityForResult(fileIntent, FETCH_ZIP_CODE);
        });

        prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());

        mSwipeRefreshLayout.setOnRefreshListener(() -> {
            recyclerView.setVisibility(View.GONE);
            new Async.LoadModules(getActivity()).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
            new UpdateUI().executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
            prefs.edit().putBoolean("ignoreUpdateAlerts", false).apply();

        });

        prefs.registerOnSharedPreferenceChangeListener((sharedPreferences, s) -> {
            if (s.contains("updated")) {
                viewMain.invalidate();
                viewMain.requestLayout();
            }
        });

        new UpdateUI().executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
        return viewMain;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (data != null) {
            // Get the URI of the selected file
            final Uri uri = data.getData();
            try {
                // Get the file path from the URI
                new Async.FlashZIP(getActivity(), uri).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
                FileInfo fileInfo = FileUtils.getFileInfo(getActivity(), uri);

            } catch (Exception e) {
                Log.e("FileSelectorTestAc...", "File select error", e);
            }
        }

    }

    @Override
    public void onResume() {
        super.onResume();
        getActivity().setTitle("Modules");
    }

    private class UpdateUI extends AsyncTask<Void, Void, Void> {

        // Just for blocking
        @Override
        protected Void doInBackground(Void... voids) {
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            super.onPostExecute(v);

            if (listModules.size() == 0) {
                emptyTv.setVisibility(View.VISIBLE);
                recyclerView.setVisibility(View.GONE);
            } else {
                recyclerView.setVisibility(View.VISIBLE);
            }
            recyclerView.setAdapter(new ModulesAdapter(listModules, (chk, position) -> {
                // On Checkbox change listener
                CheckBox chbox = (CheckBox) chk;

                if (!chbox.isChecked()) {
                    listModules.get(position).createDisableFile();
                    Snackbar.make(chk, R.string.disable_file_created, Snackbar.LENGTH_SHORT).show();
                } else {
                    listModules.get(position).removeDisableFile();
                    Snackbar.make(chk, R.string.disable_file_removed, Snackbar.LENGTH_SHORT).show();
                }
            }, (deleteBtn, position) -> {
                // On delete button click listener
                listModules.get(position).createRemoveFile();
                Snackbar.make(deleteBtn, R.string.remove_file_created, Snackbar.LENGTH_SHORT).show();
            }, (undeleteBtn, position) -> {
                // On undelete button click listener
                listModules.get(position).deleteRemoveFile();
                Snackbar.make(undeleteBtn, R.string.remove_file_deleted, Snackbar.LENGTH_SHORT).show();
            }));

            mSwipeRefreshLayout.setRefreshing(false);

        }
    }

}
