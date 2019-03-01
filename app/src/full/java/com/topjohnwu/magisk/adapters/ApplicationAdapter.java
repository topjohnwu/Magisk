package com.topjohnwu.magisk.adapters;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.ComponentInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.internal.UiThreadHandler;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Set;

import androidx.annotation.NonNull;
import androidx.annotation.WorkerThread;
import androidx.collection.ArraySet;
import androidx.recyclerview.widget.RecyclerView;
import butterknife.BindView;

public class ApplicationAdapter extends RecyclerView.Adapter<ApplicationAdapter.ViewHolder> {

    /* A list of apps that should not be shown as hide-able */
    private static final List<String> HIDE_BLACKLIST =  Arrays.asList(
            App.self.getPackageName(),
            "android",
            "com.android.chrome"
    );
    private static final String SAFETYNET_PROCESS = "com.google.android.gms.unstable";
    private static final String GMS_PACKAGE = "com.google.android.gms";

    private List<HideAppInfo> fullList, showList;
    private List<HideTarget> hideList;
    private PackageManager pm;
    private boolean showSystem;

    public ApplicationAdapter(Context context) {
        showList = Collections.emptyList();
        fullList = new ArrayList<>();
        hideList = new ArrayList<>();
        pm = context.getPackageManager();
        showSystem = Config.get(Config.Key.SHOW_SYSTEM_APP);
        AsyncTask.SERIAL_EXECUTOR.execute(this::loadApps);
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_app, parent, false);
        return new ViewHolder(v);
    }

    private void addProcesses(Set<String> set, ComponentInfo[] infos) {
        if (infos != null)
            for (ComponentInfo info : infos)
                set.add(info.processName);
    }

    @WorkerThread
    private void loadApps() {
        // Get package info with all components
        List<PackageInfo> installed = pm.getInstalledPackages(
                PackageManager.GET_ACTIVITIES | PackageManager.GET_SERVICES |
                PackageManager.GET_RECEIVERS | PackageManager.GET_PROVIDERS);

        fullList.clear();
        hideList.clear();

        for (String line : Shell.su("magiskhide --ls").exec().getOut())
            hideList.add(new HideTarget(line));

        for (PackageInfo pkg : installed) {
            if (!HIDE_BLACKLIST.contains(pkg.packageName) &&
                    /* Do not show disabled apps */
                    pkg.applicationInfo.enabled) {
                // Add all possible process names
                Set<String> procSet = new ArraySet<>();
                addProcesses(procSet, pkg.activities);
                addProcesses(procSet, pkg.services);
                addProcesses(procSet, pkg.receivers);
                addProcesses(procSet, pkg.providers);
                for (String proc : procSet) {
                    fullList.add(new HideAppInfo(pkg.applicationInfo, proc));
                }
            }
        }

        Collections.sort(fullList);
        Topic.publish(false, Topic.MAGISK_HIDE_DONE);
    }

    public void setShowSystem(boolean b) {
        showSystem = b;
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        HideAppInfo target = showList.get(position);

        holder.appIcon.setImageDrawable(target.info.loadIcon(pm));
        holder.appName.setText(target.name);
        holder.process.setText(target.process);
        if (!target.info.packageName.equals(target.process)) {
            holder.appPackage.setVisibility(View.VISIBLE);
            holder.appPackage.setText("(" + target.info.packageName + ")");
        } else {
            holder.appPackage.setVisibility(View.GONE);
        }

        holder.checkBox.setOnCheckedChangeListener(null);
        holder.checkBox.setChecked(target.hidden);
        if (target.process.equals(SAFETYNET_PROCESS)) {
            // Do not allow user to not hide SafetyNet
            holder.checkBox.setOnCheckedChangeListener((v, c) -> holder.checkBox.setChecked(true));
        } else {
            holder.checkBox.setOnCheckedChangeListener((v, isChecked) -> {
                String pair = Utils.fmt("%s %s", target.info.packageName, target.process);
                if (isChecked) {
                    Shell.su("magiskhide --add " + pair).submit();
                    target.hidden = true;
                } else {
                    Shell.su("magiskhide --rm " + pair).submit();
                    target.hidden = false;
                }
            });
        }
    }

    @Override
    public int getItemCount() {
        return showList.size();
    }

    // True if not system app and have launch intent, or user already hidden it
    private boolean systemFilter(HideAppInfo target) {
        return showSystem || target.hidden ||
                ((target.info.flags & ApplicationInfo.FLAG_SYSTEM) == 0 &&
                pm.getLaunchIntentForPackage(target.info.packageName) != null);
    }

    private boolean contains(String s, String filter) {
        return s.toLowerCase().contains(filter);
    }

    private boolean nameFilter(HideAppInfo target, String filter) {
        if (filter == null || filter.isEmpty())
            return true;
        filter = filter.toLowerCase();
        return contains(target.name, filter) ||
                contains(target.process, filter) ||
                contains(target.info.packageName, filter);
    }

    public void filter(String constraint) {
        AsyncTask.SERIAL_EXECUTOR.execute(() -> {
            List<HideAppInfo> newList = new ArrayList<>();
            for (HideAppInfo target : fullList)
                if (systemFilter(target) && nameFilter(target, constraint))
                    newList.add(target);
            UiThreadHandler.run(() -> {
                showList = newList;
                notifyDataSetChanged();
            });
        });
    }

    public void refresh() {
        AsyncTask.SERIAL_EXECUTOR.execute(this::loadApps);
    }

    static class ViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.app_icon) ImageView appIcon;
        @BindView(R.id.app_name) TextView appName;
        @BindView(R.id.process) TextView process;
        @BindView(R.id.package_name) TextView appPackage;
        @BindView(R.id.checkbox) CheckBox checkBox;

        ViewHolder(View itemView) {
            super(itemView);
            new ApplicationAdapter$ViewHolder_ViewBinding(this, itemView);
        }
    }

    class HideAppInfo implements Comparable<HideAppInfo> {
        String process;
        String name;
        ApplicationInfo info;
        boolean hidden;

        HideAppInfo(ApplicationInfo info, String process) {
            this.process = process;
            this.info = info;
            name = Utils.getAppLabel(info, pm);
            for (HideTarget tgt : hideList) {
                if (tgt.process.equals(process)) {
                    hidden = true;
                    break;
                }
            }
        }

        @Override
        public int compareTo(HideAppInfo o) {
            return Comparator.comparing((HideAppInfo t) -> t.hidden)
                    .reversed()
                    .thenComparing((a, b) -> a.name.compareToIgnoreCase(b.name))
                    .thenComparing(t -> t.info.packageName)
                    .thenComparing(t -> t.process)
                    .compare(this, o);
        }
    }

    class HideTarget {
        String pkg;
        String process;

        HideTarget(String line) {
            String[] split = line.split("\\|");
            pkg = split[0];
            if (split.length >= 2) {
                process = split[1];
            } else {
                // Backwards compatibility
                process = pkg.equals(GMS_PACKAGE) ? SAFETYNET_PROCESS : pkg;
            }
        }
    }
}
