package com.topjohnwu.magisk.ui;

import android.app.Activity;
import android.app.ProgressDialog;
import android.os.AsyncTask;
import android.os.Bundle;
import android.widget.ListView;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.model.Module;
import com.topjohnwu.magisk.rv.ModulesAdapter;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class ModulesActivity extends Activity {

    private static final String MAGISK_PATH = "/magisk";
    private static final String MAGISK_CACHE_PATH = "/cache/magisk";

    private List<Module> listModules = new ArrayList<>();
    private ListView mListView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_modules);

        mListView = (ListView) findViewById(android.R.id.list);

        new CheckFolders().execute();
    }

    private class CheckFolders extends AsyncTask<Void, Integer, Boolean> {

        private ProgressDialog progress;

        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            progress = ProgressDialog.show(ModulesActivity.this, null, getString(R.string.loading), true, false);
        }

        @Override
        protected Boolean doInBackground(Void... voids) {
            File[] magisk = new File(MAGISK_PATH).listFiles(File::isDirectory);
            File[] magiskCache = new File(MAGISK_CACHE_PATH).listFiles(File::isDirectory);

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

            mListView.setAdapter(new ModulesAdapter(ModulesActivity.this, R.layout.row, listModules));
        }
    }

}
