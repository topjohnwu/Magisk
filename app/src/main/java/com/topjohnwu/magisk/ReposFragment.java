package com.topjohnwu.magisk;

import android.app.Fragment;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.app.AlertDialog;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Logger;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ReposFragment extends Fragment {

    public static List<Repo> mListRepos = new ArrayList<>();

    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    @BindView(R.id.empty_rv) TextView emptyTv;
    @BindView(R.id.swipeRefreshLayout) SwipeRefreshLayout mSwipeRefreshLayout;

    private View mView;
    private boolean mCanUpdate;
    private boolean alertUpdate;
    private boolean ignoreAlertUpdate;
    private String alertPackage;
    private AlertDialog.Builder builder;
    private SharedPreferences.OnSharedPreferenceChangeListener listener;
    private SharedPreferences prefs;

    @Nullable

    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        mView = inflater.inflate(R.layout.repos_fragment, container, false);
        ButterKnife.bind(this, mView);

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

        //LoadRepo(false);
//        if (mListRepos.size() == 0) {
//            emptyTv.setVisibility(View.VISIBLE);
//            recyclerView.setVisibility(View.GONE);
//            return view;
//        }
        //CheckForUpdates();
        //recyclerView.setAdapter(new ReposAdapter(this, mListRepos));

        return mView;
    }

//    private void CheckForUpdates() {
//        for (int i = 0; i < mListRepos.size(); i++) {
//            if (mListRepos.get(i).canUpdate()) {
//                alertUpdate = true;
//                mListReposToUpdate.add(mListRepos.get(i));
//
//            }
//        }
//    }

    @Override
    public void onAttachFragment(Fragment childFragment) {
        super.onAttachFragment(childFragment);
    }

//    private void LoadRepo(boolean doReload) {
//        RepoHelper.TaskDelegate taskDelegate = result -> {
//            if (result.equals("Complete")) {
//                Log.d("Magisk", "ReposFragment, got delegate");
//                UpdateUI();
//                if (mView != null) {
//                    mView.invalidate();
//                    mView.requestLayout();
//                }
//
//            }
//
//        };
//        Log.d("Magisk", "ReposFragment, LoadRepo called");
//        new Async.LoadRepos(getActivity());
//    }

//    private void NotifyOfAlerts() {
//        if (alertUpdate && !ignoreAlertUpdate) {
//            Iterator<Repo> iterRepo = mListReposToUpdate.iterator();
//            while (iterRepo.hasNext()) {
//                Repo repo = iterRepo.next();
//                DialogInterface.OnClickListener dialogClickListener = (dialog, which) -> {
//                    switch (which) {
//                        case DialogInterface.BUTTON_POSITIVE:
//                            Utils.DownloadReceiver receiver = new Utils.DownloadReceiver() {
//                                @Override
//                                public void task(File file) {
//                                    Log.d("Magisk", "Task firing");
//                                    new Async.FlashZIP(getActivity(), repo.getId(), file.toString()).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
//                                }
//                            };
//                            String filename = repo.getId().replace(" ", "") + ".zip";
//                            Utils.downloadAndReceive(getActivity(), receiver, repo.getZipUrl(), filename);
//
//                            break;
//
//                        case DialogInterface.BUTTON_NEGATIVE:
////                            ignoreAlertUpdate = true;
////                            SharedPreferences.Editor editor = prefs.edit();
////                            editor.putBoolean("ignoreUpdateAlerts", ignoreAlertUpdate);
////                            editor.apply();
//                            break;
//                    }
//                };
//
//                String theme = PreferenceManager.getDefaultSharedPreferences(getActivity()).getString("theme", "");
//                Logger.dev("ReposFragment: Theme is " + theme);
//                if (theme.equals("Dark")) {
//                    builder = new AlertDialog.Builder(getActivity(),R.style.AlertDialog_dh);
//                } else {
//                    builder = new AlertDialog.Builder(getActivity());
//                }
//                    builder.setMessage("An update is available for " + repo.getName() + ".  Would you like to install it?").setPositiveButton("Yes", dialogClickListener)
//                            .setNegativeButton("No", dialogClickListener).show();
//                    iterRepo.remove();
//
//            }
//
//        }
//    }

    @Override
    public void onResume() {
        super.onResume();
        getActivity().setTitle(R.string.downloads);
        prefs.registerOnSharedPreferenceChangeListener(listener);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        prefs.unregisterOnSharedPreferenceChangeListener(listener);
    }

    private void updateUI() {
        if (mListRepos.size() == 0) {
            emptyTv.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
        } else {
            recyclerView.setVisibility(View.VISIBLE);
            recyclerView.setAdapter(new ReposAdapter(mListRepos));
        }
        mSwipeRefreshLayout.setRefreshing(false);
    }

}
