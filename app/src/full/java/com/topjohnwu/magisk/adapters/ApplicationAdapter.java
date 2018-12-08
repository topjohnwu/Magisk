package com.topjohnwu.magisk.adapters;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.Filter;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import butterknife.BindView;

public class ApplicationAdapter extends RecyclerView.Adapter<ApplicationAdapter.ViewHolder> {

    private List<ApplicationInfo> fullList, showList;
    private List<String> hideList;
    private PackageManager pm;
    private ApplicationFilter filter;

    public ApplicationAdapter(Context context) {
        fullList = showList = Collections.emptyList();
        hideList = Collections.emptyList();
        filter = new ApplicationFilter();
        pm = context.getPackageManager();
        AsyncTask.THREAD_POOL_EXECUTOR.execute(this::loadApps);
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_app, parent, false);
        return new ViewHolder(v);
    }

    private void loadApps() {
        fullList = pm.getInstalledApplications(0);
        hideList = Shell.su("magiskhide --ls").exec().getOut();
        for (Iterator<ApplicationInfo> i = fullList.iterator(); i.hasNext(); ) {
            ApplicationInfo info = i.next();
            if (Const.HIDE_BLACKLIST.contains(info.packageName) || !info.enabled || info.uid == 1000) {
                i.remove();
            }
        }
        Collections.sort(fullList, (a, b) -> {
            boolean ah = hideList.contains(a.packageName);
            boolean bh = hideList.contains(b.packageName);
            if (ah == bh) {
                return Utils.getAppLabel(a, pm).toLowerCase()
                        .compareTo(Utils.getAppLabel(b, pm).toLowerCase());
            } else if (ah) {
                return -1;
            } else {
                return 1;
            }
        });
        Topic.publish(false, Topic.MAGISK_HIDE_DONE);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        ApplicationInfo info = showList.get(position);

        holder.appIcon.setImageDrawable(info.loadIcon(pm));
        holder.appName.setText(Utils.getAppLabel(info, pm));
        holder.appPackage.setText(info.packageName);

        holder.checkBox.setOnCheckedChangeListener(null);
        holder.checkBox.setChecked(hideList.contains(info.packageName));
        holder.checkBox.setOnCheckedChangeListener((v, isChecked) -> {
            if (isChecked) {
                Shell.su("magiskhide --add " + info.packageName).submit();
                hideList.add(info.packageName);
            } else {
                Shell.su("magiskhide --rm " + info.packageName).submit();
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
        AsyncTask.THREAD_POOL_EXECUTOR.execute(this::loadApps);
    }

    static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.app_icon) ImageView appIcon;
        @BindView(R.id.app_name) TextView appName;
        @BindView(R.id.package_name) TextView appPackage;
        @BindView(R.id.checkbox) CheckBox checkBox;

        ViewHolder(View itemView) {
            super(itemView);
            new ApplicationAdapter$ViewHolder_ViewBinding(this, itemView);
        }
    }

    class ApplicationFilter extends Filter {

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
                    if (lowercaseContains(Utils.getAppLabel(info, pm), filter)
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
}
