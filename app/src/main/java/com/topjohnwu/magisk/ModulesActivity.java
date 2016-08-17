package com.topjohnwu.magisk;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

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
                    listModules.add(new Module(mod));
                }
            }

            if (magiskCache != null) {
                for (File mod : magiskCache) {
                    listModules.add(new Module(mod));
                }
            }

            //noinspection Convert2streamapi
            for (Module module : listModules) {
                if (module.isValid()) try {
                    module.parse();
                } catch (Exception ignored) {
                }
            }

            return true;
        }

        @Override
        protected void onPostExecute(Boolean aBoolean) {
            super.onPostExecute(aBoolean);

            progress.dismiss();

            mListView.setAdapter(new ModulesAdapter(ModulesActivity.this, R.layout.row));
        }
    }

    private class ModulesAdapter extends ArrayAdapter<Module> {

        public ModulesAdapter(Context context, int resource) {
            super(context, resource);
        }

        @SuppressLint("SetTextI18n")
        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder vh;

            if (convertView == null) {
                LayoutInflater inflater = (LayoutInflater) getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                convertView = inflater.inflate(R.layout.row, null);

                vh = new ViewHolder();
                vh.name = (TextView) convertView.findViewById(R.id.name);
                vh.version = (TextView) convertView.findViewById(R.id.version);
                vh.versionCode = (TextView) convertView.findViewById(R.id.versionCode);
                vh.description = (TextView) convertView.findViewById(R.id.description);
                vh.cache = (TextView) convertView.findViewById(R.id.cache);

                convertView.setTag(vh);
            } else {
                vh = (ViewHolder) convertView.getTag();
            }

            Module module = getItem(position);
            vh.name.setText("name= " + module.getName());
            vh.version.setText("version= " + module.getVersion());
            vh.versionCode.setText("versioncode= " + module.getVersionCode());
            vh.description.setText("description= " + module.getDescription());
            vh.cache.setText("is from cache= " + module.isCache());

            return convertView;
        }

        private class ViewHolder {
            public TextView name;
            public TextView version;
            public TextView versionCode;
            public TextView description;
            public TextView cache;
        }
    }
}
