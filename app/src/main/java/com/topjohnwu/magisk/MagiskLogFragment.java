package com.topjohnwu.magisk;

import android.Manifest;
import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Environment;
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
import android.widget.Toast;

import com.topjohnwu.magisk.asyncs.ParallelTask;
import com.topjohnwu.magisk.components.Fragment;
import com.topjohnwu.magisk.components.SnackbarMaker;
import com.topjohnwu.magisk.utils.Shell;
import com.topjohnwu.magisk.utils.Utils;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Calendar;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class MagiskLogFragment extends Fragment {

    private static final String MAGISK_LOG = "/cache/magisk.log";

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

        new LogManager().read();

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
        new LogManager().read();
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
                new LogManager().read();
                return true;
            case R.id.menu_save:
                Utils.runWithPermission(getActivity(),
                        Manifest.permission.WRITE_EXTERNAL_STORAGE,
                        () -> new LogManager().save());
                return true;
            case R.id.menu_clear:
                new LogManager().clear();
                return true;
            default:
                return true;
        }
    }

    private class LogManager extends ParallelTask<Object, Void, Object> {

        private int mode;
        private File targetFile;

        LogManager() {
            super(MagiskLogFragment.this.getActivity());
        }

        @SuppressLint("DefaultLocale")
        @Override
        protected Object doInBackground(Object... params) {
            mode = (int) params[0];
            switch (mode) {
                case 0:
                    StringBuildingList logList = new StringBuildingList();
                    getShell().su(logList, "cat " + MAGISK_LOG);
                    return logList.toString();

                case 1:
                    getShell().su_raw("echo -n > " + MAGISK_LOG);
                    SnackbarMaker.make(txtLog, R.string.logs_cleared, Snackbar.LENGTH_SHORT).show();
                    return "";

                case 2:
                    Calendar now = Calendar.getInstance();
                    String filename = String.format(
                            "magisk_%s_%04d%02d%02d_%02d%02d%02d.log", "error",
                            now.get(Calendar.YEAR), now.get(Calendar.MONTH) + 1,
                            now.get(Calendar.DAY_OF_MONTH), now.get(Calendar.HOUR_OF_DAY),
                            now.get(Calendar.MINUTE), now.get(Calendar.SECOND));

                    targetFile = new File(Environment.getExternalStorageDirectory() + "/MagiskManager/" + filename);

                    if ((!targetFile.getParentFile().exists() && !targetFile.getParentFile().mkdirs())
                            || (targetFile.exists() && !targetFile.delete())) {
                        return false;
                    }

                    try (FileWriter out = new FileWriter(targetFile)) {
                        FileWritingList fileWritingList = new FileWritingList(out);
                        getShell().su(fileWritingList, "cat " + MAGISK_LOG);
                    } catch (IOException e) {
                        e.printStackTrace();
                        return false;
                    }
                    return true;
            }
            return null;
        }

        @Override
        protected void onPostExecute(Object o) {
            if (o == null) return;
            switch (mode) {
                case 0:
                case 1:
                    String llog = (String) o;
                    progressBar.setVisibility(View.GONE);
                    if (TextUtils.isEmpty(llog))
                        txtLog.setText(R.string.log_is_empty);
                    else
                        txtLog.setText(llog);
                    svLog.post(() -> svLog.scrollTo(0, txtLog.getHeight()));
                    hsvLog.post(() -> hsvLog.scrollTo(0, 0));
                    break;
                case 2:
                    boolean bool = (boolean) o;
                    if (bool) {
                        getMagiskManager().toast(targetFile.toString(), Toast.LENGTH_LONG);
                    } else {
                        getMagiskManager().toast(R.string.logs_save_failed, Toast.LENGTH_LONG);
                    }
                    break;
            }
        }

        void read() {
            exec(0);
        }

        void clear() {
            exec(1);
        }

        void save() {
            exec(2);
        }
    }

    private static class StringBuildingList extends Shell.AbstractList<String> {

        StringBuilder builder;

        StringBuildingList() {
            builder = new StringBuilder();
        }

        @Override
        public boolean add(String s) {
            builder.append(s).append("\n");
            return true;
        }

        @Override
        public String toString() {
            return builder.toString();
        }
    }

    private static class FileWritingList extends Shell.AbstractList<String> {

        private FileWriter writer;

        FileWritingList(FileWriter out) {
            writer = out;
        }

        @Override
        public boolean add(String s) {
            try {
                writer.write(s + "\n");
            } catch (IOException ignored) {}
            return true;
        }
    }

}
