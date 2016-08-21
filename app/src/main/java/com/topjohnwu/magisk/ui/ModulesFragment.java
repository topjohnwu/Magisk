package com.topjohnwu.magisk.ui;

import android.app.ProgressDialog;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.model.Module;
import com.topjohnwu.magisk.rv.ModulesAdapter;
import com.topjohnwu.magisk.ui.utils.Utils;

import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesFragment extends android.support.v4.app.Fragment {

    private static final String MAGISK_PATH = "/magisk";
    private static final String MAGISK_CACHE_PATH = "/cache/magisk";

    @BindView(R.id.recyclerView) RecyclerView recyclerView;

    private List<Module> listModules = new ArrayList<>();

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.modules_fragment, container, false);
        ButterKnife.bind(this, v);

        new CheckFolders().execute();

        return v;
    }

    private class CheckFolders extends AsyncTask<Void, Integer, Boolean> {

        private ProgressDialog progress;

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            progress = ProgressDialog.show(getContext(), null, getString(R.string.loading), true, false);
        }

        @Override
        protected Boolean doInBackground(Void... voids) {
            File[] magisk = new File(MAGISK_PATH).listFiles(new FileFilter() {
                @Override
                public boolean accept(File file) {
                    return file.isDirectory();
                }
            });

            Utils.executeCommand("chmod 777 /cache");

            File[] magiskCache = new File(MAGISK_CACHE_PATH).listFiles(new FileFilter() {
                @Override
                public boolean accept(File file) {
                    return file.isDirectory();
                }
            });

            if (magisk != null) {
                for (File mod : magisk) {
                    Module m = new Module(mod);
                    if (m.isValid()) {
                        listModules.add(m);
                    }
                }
            }

            if (magiskCache != null) {
                for (File mod : magiskCache) {
                    Module m = new Module(mod);
                    if (m.isValid()) {
                        listModules.add(m);
                    }
                }
            }

            //noinspection Convert2streamapi
            for (Module module : listModules) {
                try {
                    module.parse();
                } catch (Exception ignored) {
                }
            }

            return true;
        }

        @Override
        protected void onPostExecute(Boolean result) {
            super.onPostExecute(result);

            progress.dismiss();

            recyclerView.setAdapter(new ModulesAdapter(listModules));
        }
    }

}
