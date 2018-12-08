package com.topjohnwu.magisk.fragments;

import android.Manifest;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.HorizontalScrollView;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.TextView;

import com.google.android.material.snackbar.Snackbar;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.components.BaseFragment;
import com.topjohnwu.magisk.components.SnackbarMaker;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import java.io.File;
import java.io.IOException;
import java.util.Calendar;

import androidx.annotation.Nullable;
import butterknife.BindView;

public class MagiskLogFragment extends BaseFragment {

    @BindView(R.id.txtLog) TextView txtLog;
    @BindView(R.id.svLog) ScrollView svLog;
    @BindView(R.id.hsvLog) HorizontalScrollView hsvLog;
    @BindView(R.id.progressBar) ProgressBar progressBar;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_magisk_log, container, false);
        unbinder = new MagiskLogFragment_ViewBinding(this, view);
        setHasOptionsMenu(true);
        txtLog.setTextIsSelectable(true);
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
                return true;
            default:
                return true;
        }
    }

    private void readLogs() {
        Shell.su("cat " + Const.MAGISK_LOG + " | tail -n 5000").submit(result -> {
            progressBar.setVisibility(View.GONE);
            if (result.getOut().isEmpty())
                txtLog.setText(R.string.log_is_empty);
            else
                txtLog.setText(TextUtils.join("\n", result.getOut()));
            svLog.postDelayed(() -> svLog.fullScroll(ScrollView.FOCUS_DOWN), 100);
            hsvLog.postDelayed(() -> hsvLog.fullScroll(ScrollView.FOCUS_LEFT), 100);
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
                        SnackbarMaker.make(txtLog, logFile.getPath(), Snackbar.LENGTH_SHORT).show());
    }

    private void clearLogs() {
        Shell.su("echo -n > " + Const.MAGISK_LOG).submit();
        txtLog.setText(R.string.log_is_empty);
        SnackbarMaker.make(txtLog, R.string.logs_cleared, Snackbar.LENGTH_SHORT).show();
    }
}
