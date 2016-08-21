package com.topjohnwu.magisk.ui;

import android.os.AsyncTask;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.design.widget.Snackbar;
import android.support.v7.widget.PopupMenu;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ProgressBar;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.model.Module;
import com.topjohnwu.magisk.rv.ItemClickListener;
import com.topjohnwu.magisk.rv.ModulesAdapter;
import com.topjohnwu.magisk.ui.utils.Utils;

import java.io.File;
import java.io.FileFilter;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ModulesFragment extends android.support.v4.app.Fragment {

    private static final String MAGISK_PATH = "/magisk";
    private static final String MAGISK_CACHE_PATH = "/cache/magisk";

    @BindView(R.id.recyclerView) RecyclerView recyclerView;
    @BindView(R.id.progressBar) ProgressBar progressBar;

    private List<Module> listModules = new ArrayList<>();

    private ItemClickListener moduleActions = new ItemClickListener() {
        @Override
        public void onItemClick(final View view, final int position) {
            PopupMenu popup = new PopupMenu(getContext(), view);
            try {
                Field[] fields = popup.getClass().getDeclaredFields();
                for (Field field : fields) {
                    if ("mPopup".equals(field.getName())) {
                        field.setAccessible(true);
                        Object menuPopupHelper = field.get(popup);
                        Class<?> classPopupHelper = Class.forName(menuPopupHelper.getClass().getName());
                        Method setForceIcons = classPopupHelper.getMethod("setForceShowIcon", boolean.class);
                        setForceIcons.invoke(menuPopupHelper, true);
                        break;
                    }
                }
            } catch (Exception ignored) {
            }

            popup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                @Override
                public boolean onMenuItemClick(MenuItem item) {
                    switch (item.getItemId()) {
                        case R.id.remove:
                            listModules.get(position).createRemoveFile();
                            Snackbar.make(view, R.string.remove_file_created, Snackbar.LENGTH_SHORT).show();
                            break;
                        case R.id.disable:
                            listModules.get(position).createDisableFile();
                            Snackbar.make(view, R.string.disable_file_created, Snackbar.LENGTH_SHORT).show();
                            break;
                    }

                    return false;
                }
            });
            popup.inflate(R.menu.module_popup);
            popup.show();
        }
    };

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.modules_fragment, container, false);
        ButterKnife.bind(this, view);

        new CheckFolders().execute();

        return view;
    }

    private class CheckFolders extends AsyncTask<Void, Integer, Boolean> {

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

            progressBar.setVisibility(View.GONE);

            recyclerView.setAdapter(new ModulesAdapter(listModules, moduleActions));
        }
    }

}
