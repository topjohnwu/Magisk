package com.topjohnwu.magisk;

import android.Manifest;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.design.widget.Snackbar;
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

import com.topjohnwu.magisk.components.Fragment;
import com.topjohnwu.magisk.components.SnackbarMaker;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;

import java.io.File;
import java.io.IOException;
import java.util.Calendar;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class MagiskLogFragment extends Fragment {

    private Unbinder unbinder;

    @BindView(R.id.txtLog) TextView txtLog;
    @BindView(R.id.svLog) ScrollView svLog;
    @BindView(R.id.hsvLog) HorizontalScrollView hsvLog;
    @BindView(R.id.progressBar) ProgressBar progressBar;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_magisk_log, container, false);
        unbinder = ButterKnife.bind(this, view);
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
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
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

    public void readLogs() {
        Shell.Async.su(new Shell.Async.Callback() {
            @Override
            public void onTaskResult(@Nullable List<String> out, @Nullable List<String> err) {
                progressBar.setVisibility(View.GONE);
                if (ShellUtils.isValidOutput(out)) {
                    txtLog.setText(TextUtils.join("\n", out));
                } else {
                    txtLog.setText(R.string.log_is_empty);
                }
                svLog.postDelayed(() -> svLog.fullScroll(ScrollView.FOCUS_DOWN), 100);
                hsvLog.postDelayed(() -> hsvLog.fullScroll(ScrollView.FOCUS_LEFT), 100);
            }

            @Override
            public void onTaskError(@NonNull Throwable throwable) {
                txtLog.setText(R.string.log_is_empty);
            }
        }, "cat " + Const.MAGISK_LOG + " | tail -n 5000");
    }

    public void saveLogs() {
        Calendar now = Calendar.getInstance();
        String filename = Utils.fmt("magisk_log_%04d%02d%02d_%02d%02d%02d.log",
                now.get(Calendar.YEAR), now.get(Calendar.MONTH) + 1,
                now.get(Calendar.DAY_OF_MONTH), now.get(Calendar.HOUR_OF_DAY),
                now.get(Calendar.MINUTE), now.get(Calendar.SECOND));

        File targetFile = new File(Const.EXTERNAL_PATH + "/logs", filename);
        targetFile.getParentFile().mkdirs();
        try {
            targetFile.createNewFile();
        } catch (IOException e) {
            return;
        }
        Shell.Async.su(new Shell.Async.Callback() {
            @Override
            public void onTaskResult(@Nullable List<String> out, @Nullable List<String> err) {
                SnackbarMaker.make(txtLog, targetFile.getPath(), Snackbar.LENGTH_SHORT).show();
            }

            @Override
            public void onTaskError(@NonNull Throwable throwable) {}
        }, "cat " + Const.MAGISK_LOG + " > " + targetFile);
    }

    public void clearLogs() {
        Shell.Async.su("echo -n > " + Const.MAGISK_LOG);
        SnackbarMaker.make(txtLog, R.string.logs_cleared, Snackbar.LENGTH_SHORT).show();
    }
}
