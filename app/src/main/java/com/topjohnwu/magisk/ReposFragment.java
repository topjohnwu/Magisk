package com.topjohnwu.magisk;

import android.app.Fragment;
import android.content.DialogInterface;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.Nullable;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.app.AlertDialog;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.topjohnwu.magisk.module.Repo;
import com.topjohnwu.magisk.module.RepoHelper;
import com.topjohnwu.magisk.utils.Async;
import com.topjohnwu.magisk.utils.Logger;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ReposFragment extends Fragment {

    public static List<Repo> mListRepos = new ArrayList<>();
    public static List<Repo> mListReposToUpdate = new ArrayList<>();
    @BindView(R.id.recyclerView)
    RecyclerView recyclerView;
    @BindView(R.id.empty_rv)
    TextView emptyTv;
    @BindView(R.id.swipeRefreshLayout)
    SwipeRefreshLayout swipeRefreshLayout;
    private View mView;
    private boolean mCanUpdate;
    private boolean alertUpdate;
    private boolean ignoreAlertUpdate;
    private String alertPackage;
    private AlertDialog.Builder builder;
//    private SharedPreferences prefs;

    @Nullable

    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.repos_fragment, container, false);
        mView = view;
        ButterKnife.bind(this, view);
        swipeRefreshLayout.setOnRefreshListener(() -> {
            new Async.LoadRepos(getActivity()).executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
            new UpdateUI().executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
            ignoreAlertUpdate = false;
        });
        //LoadRepo(false);
        new UpdateUI().executeOnExecutor(AsyncTask.SERIAL_EXECUTOR);
        setHasOptionsMenu(false);
        alertUpdate = false;
//        if (mListRepos.size() == 0) {
//            emptyTv.setVisibility(View.VISIBLE);
//            recyclerView.setVisibility(View.GONE);
//            return view;
//        }
        //CheckForUpdates();
        //recyclerView.setAdapter(new ReposAdapter(this, mListRepos));

        return view;
    }

    private void CheckForUpdates() {
        for (int i = 0; i < mListRepos.size(); i++) {
            if (mListRepos.get(i).canUpdate()) {
                alertUpdate = true;
                mListReposToUpdate.add(mListRepos.get(i));

            }
        }
    }

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
//                Logger.dh("ReposFragment: Theme is " + theme);
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
        //LoadRepo(false);
        getActivity().setTitle("Magisk");

    }
    
    private class UpdateUI extends AsyncTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            super.onPostExecute(v);
            if (mListRepos.size() == 0) {
                emptyTv.setVisibility(View.VISIBLE);
                recyclerView.setVisibility(View.GONE);

            }
            Log.d("Magisk", "ReposFragment: ListRepos size is " + mListRepos.size());
            recyclerView.setAdapter(new ReposAdapter(mListRepos));
            if (swipeRefreshLayout.isRefreshing()) {
                swipeRefreshLayout.setRefreshing(false);
                //CheckForUpdates();
                //NotifyOfAlerts();
            }
        }
    }

//    private void UpdateUI() {
//        Log.d("Magisk", "ReposFragment: UpdateUI Called, size is " + mListRepos.size());
//
//        if (mListRepos.size() == 0) {
//            emptyTv.setVisibility(View.VISIBLE);
//            recyclerView.setVisibility(View.GONE);
//
//        }
//        Log.d("Magisk", "ReposFragment: ListRepos size is " + mListRepos.size());
//        recyclerView.setAdapter(new ReposAdapter(this, mListRepos));
//        if (swipeRefreshLayout.isRefreshing()) {
//            swipeRefreshLayout.setRefreshing(false);
//            CheckForUpdates();
//            //NotifyOfAlerts();
//        }
//
//    }

}
