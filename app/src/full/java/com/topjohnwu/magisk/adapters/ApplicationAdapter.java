package com.topjohnwu.magisk.adapters;

import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.Filter;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.MagiskManager;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.asyncs.ParallelTask;
import com.topjohnwu.magisk.utils.Const;
import com.topjohnwu.superuser.Shell;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class ApplicationAdapter extends RecyclerView.Adapter<ApplicationAdapter.ViewHolder> {

    private List<ApplicationInfo> fullList, showList;
    private List<String> hideList;
    private PackageManager pm;
    private ApplicationFilter filter;

    public ApplicationAdapter() {
        fullList = showList = Collections.emptyList();
        hideList = Collections.emptyList();
        filter = new ApplicationFilter();
        pm = MagiskManager.get().getPackageManager();
        new LoadApps().exec();
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View mView = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_app, parent, false);
        return new ViewHolder(mView);
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, int position) {
        ApplicationInfo info = showList.get(position);

        holder.appIcon.setImageDrawable(info.loadIcon(pm));
        holder.appName.setText(info.loadLabel(pm));
        holder.appPackage.setText(info.packageName);

        holder.checkBox.setOnCheckedChangeListener(null);
        holder.checkBox.setChecked(hideList.contains(info.packageName));
        holder.checkBox.setOnCheckedChangeListener((v, isChecked) -> {
            if (isChecked) {
                Shell.Async.su("magiskhide --add " + info.packageName);
                hideList.add(info.packageName);
            } else {
                Shell.Async.su("magiskhide --rm " + info.packageName);
                hideList.remove(info.packageName);
            }
        });
    }

    @Override
    public int getItemCount() {
        return showList.size();
    }

    public void filter(String constraint) {
        filter.filter(constraint);
    }

    public void refresh() {
        new LoadApps().exec();
    }

    static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.app_icon) ImageView appIcon;
        @BindView(R.id.app_name) TextView appName;
        @BindView(R.id.package_name) TextView appPackage;
        @BindView(R.id.checkbox) CheckBox checkBox;

        ViewHolder(View itemView) {
            super(itemView);
            ButterKnife.bind(this, itemView);
        }
    }

    private class ApplicationFilter extends Filter {

        private boolean lowercaseContains(String s, CharSequence filter) {
            return !TextUtils.isEmpty(s) && s.toLowerCase().contains(filter);
        }

        @Override
        protected FilterResults performFiltering(CharSequence constraint) {
            if (constraint == null || constraint.length() == 0) {
                showList = fullList;
            } else {
                showList = new ArrayList<>();
                String filter = constraint.toString().toLowerCase();
                for (ApplicationInfo info : fullList) {
                    if (lowercaseContains(info.loadLabel(pm).toString(), filter)
                            || lowercaseContains(info.packageName, filter)) {
                        showList.add(info);
                    }
                }
            }
            return null;
        }

        @Override
        protected void publishResults(CharSequence constraint, FilterResults results) {
            notifyDataSetChanged();
        }
    }

    private class LoadApps extends ParallelTask<Void, Void, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            fullList = pm.getInstalledApplications(0);
            hideList = Shell.Sync.su("magiskhide --ls");
            for (Iterator<ApplicationInfo> i = fullList.iterator(); i.hasNext(); ) {
                ApplicationInfo info = i.next();
                if (Const.HIDE_BLACKLIST.contains(info.packageName) || !info.enabled) {
                    i.remove();
                }
            }
            Collections.sort(fullList, (a, b) -> {
                boolean ah = hideList.contains(a.packageName);
                boolean bh = hideList.contains(b.packageName);
                if (ah == bh) {
                    return a.loadLabel(pm).toString().toLowerCase().compareTo(
                            b.loadLabel(pm).toString().toLowerCase());
                } else if (ah) {
                    return -1;
                } else {
                    return 1;
                }
            });
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            MagiskManager.get().magiskHideDone.publish(false);
        }
    }
}
