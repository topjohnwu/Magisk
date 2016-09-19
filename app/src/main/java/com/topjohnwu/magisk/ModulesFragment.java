package com.topjohnwu.magisk;

import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.design.widget.Snackbar;
import android.support.v4.app.Fragment;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Module;
import com.topjohnwu.magisk.utils.Utils;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesFragment extends Fragment {
    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;
    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    @BindView(R.id.empty_rv) TextView emptyTv;

    private SharedPreferences prefs;
    public static List<Module> listModules = new ArrayList<>();

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View viewMain = inflater.inflate(R.layout.modules_fragment, container, false);


        ButterKnife.bind(this, viewMain);

        prefs = PreferenceManager.getDefaultSharedPreferences(getActivity());
        mSwipeRefreshLayout.setOnRefreshListener(() -> {

            recyclerView.setVisibility(View.GONE);
            new Utils.LoadModules(getActivity()).execute();
            new updateUI().execute();
            prefs.edit().putBoolean("ignoreUpdateAlerts", false).apply();

        });

        prefs.registerOnSharedPreferenceChangeListener((sharedPreferences, s) -> {
            if (s.contains("updated")) {
                viewMain.invalidate();
                viewMain.requestLayout();

            }
        });

        new updateUI().execute();

        return viewMain;
    }

    private class updateUI extends AsyncTask<Void, Void, Void> {

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

            if (mSwipeRefreshLayout.isRefreshing())
                mSwipeRefreshLayout.setRefreshing(false);

        }
    }

}
