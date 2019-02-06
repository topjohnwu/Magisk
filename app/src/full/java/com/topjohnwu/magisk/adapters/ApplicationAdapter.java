package com.topjohnwu.magisk.adapters;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.internal.UiThreadHandler;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import androidx.annotation.NonNull;
import androidx.annotation.WorkerThread;
import androidx.recyclerview.widget.RecyclerView;
import butterknife.BindView;

public class ApplicationAdapter extends RecyclerView.Adapter<ApplicationAdapter.ViewHolder> {

    private static PackageInfo PLATFORM;

    private List<PackageInfo> fullList, showList;
    private List<String> hideList;
    private PackageManager pm;
    private boolean showSystem;

    public ApplicationAdapter(Context context) {
        fullList = showList = Collections.emptyList();
        hideList = Collections.emptyList();
        pm = context.getPackageManager();
        showSystem = false;
        if (PLATFORM == null) {
            try {
                PLATFORM = pm.getPackageInfo("android", PackageManager.GET_SIGNATURES);
            } catch (PackageManager.NameNotFoundException ignored) {}
        }
        AsyncTask.SERIAL_EXECUTOR.execute(this::loadApps);
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_app, parent, false);
        return new ViewHolder(v);
    }

    @WorkerThread
    private void loadApps() {
        fullList = pm.getInstalledPackages(PackageManager.GET_SIGNATURES);
        hideList = Shell.su("magiskhide --ls").exec().getOut();
        for (Iterator<PackageInfo> i = fullList.iterator(); i.hasNext(); ) {
            PackageInfo info = i.next();
            if (Const.HIDE_BLACKLIST.contains(info.packageName) ||
                    /* Do not show disabled apps */
                    !info.applicationInfo.enabled ||
                    /* Never show platform apps */
                    PLATFORM.signatures[0].equals(info.signatures[0])) {
                i.remove();
            }
        }
        Collections.sort(fullList, (a, b) -> {
            boolean ah = hideList.contains(a.packageName);
            boolean bh = hideList.contains(b.packageName);
            if (ah == bh) {
                return Utils.getAppLabel(a.applicationInfo, pm)
                        .compareToIgnoreCase(Utils.getAppLabel(b.applicationInfo, pm));
            } else if (ah) {
                return -1;
            } else {
                return 1;
            }
        });
        Topic.publish(false, Topic.MAGISK_HIDE_DONE);
    }

    public void setShowSystem(boolean b) {
        showSystem = b;
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        ApplicationInfo info = showList.get(position).applicationInfo;

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

    private boolean contains(String s, String filter) {
        return s.toLowerCase().contains(filter);
    }

    // Show if have launch intent or not system app
    private boolean systemFilter(PackageInfo info) {
        if (showSystem)
            return true;
        return (info.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) == 0 ||
                pm.getLaunchIntentForPackage(info.packageName) != null;
    }

    public void filter(String constraint) {
        AsyncTask.SERIAL_EXECUTOR.execute(() -> {
            showList = new ArrayList<>();
            if (constraint == null || constraint.length() == 0) {
                for (PackageInfo info : fullList) {
                    if (systemFilter(info))
                        showList.add(info);
                }
            } else {
                String filter = constraint.toLowerCase();
                for (PackageInfo info : fullList) {
                    if ((contains(Utils.getAppLabel(info.applicationInfo, pm), filter) ||
                            contains(info.packageName, filter)) && systemFilter(info)) {
                        showList.add(info);
                    }
                }
            }
            UiThreadHandler.run(this::notifyDataSetChanged);
        });
    }

    public void refresh() {
        AsyncTask.SERIAL_EXECUTOR.execute(this::loadApps);
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
}
