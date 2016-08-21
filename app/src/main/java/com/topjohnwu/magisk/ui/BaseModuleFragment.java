package com.topjohnwu.magisk.ui;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.design.widget.Snackbar;
import android.support.v4.app.Fragment;
import android.support.v7.widget.PopupMenu;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;

import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.model.Module;
import com.topjohnwu.magisk.rv.ItemClickListener;
import com.topjohnwu.magisk.rv.ModulesAdapter;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public abstract class BaseModuleFragment extends Fragment {

    @BindView(R.id.recyclerView) RecyclerView recyclerView;

    private ItemClickListener moduleActions = new ItemClickListener() {
        @Override
        public void onItemClick(final View view, final int position) {
            PopupMenu popup = new PopupMenu(getContext(), view);

            // Force show icons
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
                            listModules().get(position).createRemoveFile();
                            Snackbar.make(view, R.string.remove_file_created, Snackbar.LENGTH_SHORT).show();
                            break;
                        case R.id.disable:
                            listModules().get(position).createDisableFile();
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
        View view = inflater.inflate(R.layout.single_module_fragment, container, false);
        ButterKnife.bind(this, view);

        recyclerView.setAdapter(new ModulesAdapter(listModules(), moduleActions));
        return view;
    }

    protected abstract List<Module> listModules();
}
