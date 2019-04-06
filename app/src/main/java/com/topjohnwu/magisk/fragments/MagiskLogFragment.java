package com.topjohnwu.magisk.fragments;

import android.Manifest;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.snackbar.Snackbar;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.adapters.StringListAdapter;
import com.topjohnwu.magisk.components.BaseFragment;
import com.topjohnwu.magisk.uicomponents.SnackbarMaker;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.internal.NOPList;

import java.io.File;
import java.io.IOException;
import java.util.Calendar;
import java.util.List;

import butterknife.BindView;

public class MagiskLogFragment extends BaseFragment {

    @BindView(R.id.recyclerView) RecyclerView rv;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_magisk_log, container, false);
        unbinder = new MagiskLogFragment_ViewBinding(this, view);
        setHasOptionsMenu(true);
        return view;
    }

    @Override
    public void onStart() {
        super.onStart();
        getActivity().setTitle(R.string.log);
    }

    @Override
    public void onResume() {
        super.onResume();
        readLogs();
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.menu_log, menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_refresh:
                readLogs();
                return true;
            case R.id.menu_save:
                runWithPermission(new String[] { Manifest.permission.WRITE_EXTERNAL_STORAGE }, this::saveLogs);
                return true;
            case R.id.menu_clear:
                clearLogs();
                rv.setAdapter(new MagiskLogAdapter(NOPList.getInstance()));
                return true;
            default:
                return true;
        }
    }

    private void readLogs() {
        Shell.su("tail -n 5000 " + Const.MAGISK_LOG).submit(result -> {
            rv.setAdapter(new MagiskLogAdapter(result.getOut()));
        });
    }

    private void saveLogs() {
        Calendar now = Calendar.getInstance();
        String filename = Utils.fmt("magisk_log_%04d%02d%02d_%02d%02d%02d.log",
                now.get(Calendar.YEAR), now.get(Calendar.MONTH) + 1,
                now.get(Calendar.DAY_OF_MONTH), now.get(Calendar.HOUR_OF_DAY),
                now.get(Calendar.MINUTE), now.get(Calendar.SECOND));

        File logFile = new File(Const.EXTERNAL_PATH, filename);
        try {
            logFile.createNewFile();
        } catch (IOException e) {
            return;
        }
        Shell.su("cat " + Const.MAGISK_LOG + " > " + logFile)
                .submit(result ->
                        SnackbarMaker.make(rv, logFile.getPath(), Snackbar.LENGTH_SHORT).show());
    }

    private void clearLogs() {
        Shell.su("echo -n > " + Const.MAGISK_LOG).submit();
        SnackbarMaker.make(rv, R.string.logs_cleared, Snackbar.LENGTH_SHORT).show();
    }

    private class MagiskLogAdapter extends StringListAdapter<MagiskLogAdapter.ViewHolder> {

        MagiskLogAdapter(List<String> list) {
            super(list);
            if (mList.isEmpty())
                mList.add(requireContext().getString(R.string.log_is_empty));
        }

        @Override
        protected int itemLayoutRes() {
            return R.layout.list_item_console;
        }

        @NonNull
        @Override
        public ViewHolder createViewHolder(@NonNull View v) {
            return new ViewHolder(v);
        }

        @Override
        protected void onUpdateTextWidth(ViewHolder holder) {
            super.onUpdateTextWidth(holder);
            // Find the longest string and update accordingly
            int max = 0;
            String maxStr = "";
            for (String s : mList) {
                int len = s.length();
                if (len > max) {
                    max = len;
                    maxStr = s;
                }
            }
            holder.txt.setText(maxStr);
            super.onUpdateTextWidth(holder);
        }

        public class ViewHolder extends StringListAdapter.ViewHolder {

            public ViewHolder(@NonNull View itemView) {
                super(itemView);
            }

            @Override
            protected int textViewResId() {
                return R.id.txt;
            }
        }
    }
}
