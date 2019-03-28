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

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.WorkerThread;
import androidx.collection.ArraySet;
import androidx.recyclerview.widget.RecyclerView;

import com.buildware.widget.indeterm.IndeterminateCheckBox;
import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.R;
import com.topjohnwu.magisk.uicomponents.ArrowExpandable;
import com.topjohnwu.magisk.uicomponents.Expandable;
import com.topjohnwu.magisk.utils.Event;
import com.topjohnwu.magisk.utils.Utils;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.internal.UiThreadHandler;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Set;

import butterknife.BindView;
import java9.util.Comparators;
import java9.util.Lists;
import java9.util.Objects;
import java9.util.Sets;
import java9.util.stream.Collectors;
import java9.util.stream.Stream;
import java9.util.stream.StreamSupport;

public class ApplicationAdapter extends SectionedAdapter
        <ApplicationAdapter.AppViewHolder, ApplicationAdapter.ProcessViewHolder> {

    private static final String SAFETYNET_PROCESS = "com.google.android.gms.unstable";
    private static final String GMS_PACKAGE = "com.google.android.gms";
    private static boolean old_hide = false;

    /* A list of apps that should not be shown as hide-able */
    private static final List<String> HIDE_BLACKLIST = Lists.of(
            App.self.getPackageName(),
            "android",
            "com.android.chrome",
            "com.chrome.beta",
            "com.chrome.dev",
            "com.chrome.canary",
            "com.android.webview",
            "com.google.android.webview"
    );
    private static final List<String> DEFAULT_HIDELIST = Lists.of(
            SAFETYNET_PROCESS
    );

    private static int BOTTOM_MARGIN = -1;

    private List<HideAppInfo> fullList, showList;
    private List<HideTarget> hideList;
    private PackageManager pm;
    private boolean showSystem;

    public ApplicationAdapter(Context context) {
        fullList = showList = Collections.emptyList();
        hideList = Collections.emptyList();
        pm = context.getPackageManager();
        showSystem = Config.get(Config.Key.SHOW_SYSTEM_APP);
        AsyncTask.SERIAL_EXECUTOR.execute(this::loadApps);
    }

    private static ViewGroup.MarginLayoutParams getMargins(RecyclerView.ViewHolder vh) {
        return (ViewGroup.MarginLayoutParams) vh.itemView.getLayoutParams();
    }

    @Override
    public int getSectionCount() {
        return showList.size();
    }

    @Override
    public int getItemCount(int section) {
        HideAppInfo app = showList.get(section);
        return app.expanded ? app.processList.size() : 0;
    }

    @Override
    public AppViewHolder onCreateSectionViewHolder(ViewGroup parent) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_hide_app, parent, false);
        AppViewHolder vh = new AppViewHolder(v);
        if (BOTTOM_MARGIN < 0)
            BOTTOM_MARGIN = getMargins(vh).bottomMargin;
        return vh;
    }

    @Override
    public ProcessViewHolder onCreateItemViewHolder(ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item_hide_process, parent, false);
        return new ProcessViewHolder(v);
    }

    @Override
    public void onBindSectionViewHolder(AppViewHolder holder, int section) {
        HideAppInfo app = showList.get(section);
        holder.app_name.setText(app.name);
        holder.app_icon.setImageDrawable(app.info.loadIcon(pm));
        holder.package_name.setText(app.info.packageName);
        holder.checkBox.setOnStateChangedListener(null);
        holder.checkBox.setState(app.getState());
        holder.ex.setExpanded(app.expanded);

        int index = getItemPosition(section, 0);
        holder.checkBox.setOnStateChangedListener((IndeterminateCheckBox box, @Nullable Boolean status) -> {
            if (status != null) {
                setHide(status, app);
                if (app.expanded)
                    notifyItemRangeChanged(index, app.processList.size());
            }
        });

        if (app.processList.size() > 1) {
            holder.arrow.setVisibility(View.VISIBLE);

            holder.trigger.setOnClickListener((v) -> {
                if (app.expanded) {
                    app.expanded = false;
                    notifyItemRangeRemoved(index, app.processList.size());
                    holder.ex.collapse();
                } else {
                    app.expanded = true;
                    notifyItemRangeInserted(index, app.processList.size());
                    holder.ex.expand();
                }
            });
        } else {
            holder.arrow.setVisibility(View.GONE);
            holder.trigger.setOnClickListener(null);
        }

    }

    @Override
    public void onBindItemViewHolder(ProcessViewHolder holder, int section, int position) {
        HideAppInfo app = showList.get(section);
        HideProcessInfo target = app.processList.get(position);
        holder.process.setText(target.name);
        holder.checkbox.setOnCheckedChangeListener(null);
        holder.checkbox.setChecked(target.hidden);
        holder.checkbox.setOnCheckedChangeListener((v, checked) -> {
            setHide(checked, app, target);
            notifyItemChanged(getSectionPosition(section));
        });
        getMargins(holder).bottomMargin =
                position == app.processList.size() - 1 ? BOTTOM_MARGIN : 0;
    }

    public void filter(String constraint) {
        AsyncTask.SERIAL_EXECUTOR.execute(() -> {
            Stream<HideAppInfo> s = StreamSupport.stream(fullList)
                    .filter(this::systemFilter)
                    .filter(t -> nameFilter(t, constraint));
            UiThreadHandler.run(() -> {
                showList = s.collect(Collectors.toList());
                notifyDataSetChanged();
            });
        });
    }

    public void setShowSystem(boolean b) {
        showSystem = b;
    }

    public void refresh() {
        AsyncTask.SERIAL_EXECUTOR.execute(this::loadApps);
    }

    private void setHide(boolean add, HideAppInfo app) {
        if (add) {
            StreamSupport.stream(app.processList).forEach(p -> setHide(true, app, p));
        } else {
            if (StreamSupport.stream(app.processList)
                    .anyMatch(p -> p.name.equals(SAFETYNET_PROCESS))) {
                StreamSupport.stream(app.processList).forEach(p -> setHide(false, app, p));
            } else {
                // Quick removal
                Shell.su("magiskhide --rm " + app.info.packageName).submit();
                StreamSupport.stream(app.processList).forEach(p -> p.hidden = false);
            }
        }
    }

    private void setHide(boolean add, HideAppInfo app, HideProcessInfo process) {
        // Don't remove SafetyNet
        if (!add && DEFAULT_HIDELIST.contains(process.name))
            return;
        Shell.su(Utils.fmt("magiskhide --%s %s %s", add ? "add" : "rm",
                app.info.packageName, process.name)).submit();
        process.hidden = add;
    }

    private void addProcesses(Set<String> set, ComponentInfo[] infos) {
        if (infos != null)
            for (ComponentInfo info : infos)
                set.add(info.processName);
    }

    private PackageInfo getPackageInfo(String pkg) {
        // Try super hard to get as much info as possible
        try {
            return pm.getPackageInfo(pkg,
                    PackageManager.GET_ACTIVITIES | PackageManager.GET_SERVICES |
                            PackageManager.GET_RECEIVERS | PackageManager.GET_PROVIDERS);
        } catch (Exception e1) {
            try {
                PackageInfo info = pm.getPackageInfo(pkg, PackageManager.GET_ACTIVITIES);
                info.services = pm.getPackageInfo(pkg, PackageManager.GET_SERVICES).services;
                info.receivers = pm.getPackageInfo(pkg, PackageManager.GET_RECEIVERS).receivers;
                info.providers = pm.getPackageInfo(pkg, PackageManager.GET_PROVIDERS).providers;
                return info;
            } catch (Exception e2) {
                return null;
            }
        }
    }

    @WorkerThread
    private void loadApps() {
        hideList = StreamSupport.stream(Shell.su("magiskhide --ls").exec().getOut())
                .map(HideTarget::new)
                .collect(Collectors.toList());

        fullList = StreamSupport.stream(pm.getInstalledApplications(0))
                .filter(info -> !HIDE_BLACKLIST.contains(info.packageName) && info.enabled)
                .map(info -> {
                    if (old_hide) {
                        return new HideAppInfo(info, Sets.of(info.packageName));
                    } else {
                        Set<String> set = new ArraySet<>();
                        PackageInfo pkg = getPackageInfo(info.packageName);
                        if (pkg != null) {
                            addProcesses(set, pkg.activities);
                            addProcesses(set, pkg.services);
                            addProcesses(set, pkg.receivers);
                            addProcesses(set, pkg.providers);
                        }
                        if (set.isEmpty())
                            return null;
                        return new HideAppInfo(info, set);
                    }
                }).filter(Objects::nonNull).sorted()
                .collect(Collectors.toList());

        Event.trigger(false, Event.MAGISK_HIDE_DONE);
    }

    // True if not system app or user already hidden it
    private boolean systemFilter(HideAppInfo target) {
        return showSystem || target.haveHidden() ||
                (target.info.flags & ApplicationInfo.FLAG_SYSTEM) == 0;
    }

    private boolean contains(String s, String filter) {
        return s.toLowerCase().contains(filter);
    }

    private boolean nameFilter(HideAppInfo target, String filter) {
        if (filter == null || filter.isEmpty())
            return true;
        filter = filter.toLowerCase();
        if (contains(target.name, filter))
            return true;
        for (HideProcessInfo p : target.processList) {
            if (contains(p.name, filter))
                return true;
        }
        return contains(target.info.packageName, filter);
    }

    class HideAppInfo implements Comparable<HideAppInfo> {
        String name;
        ApplicationInfo info;
        List<HideProcessInfo> processList;
        boolean expanded;
        IndeterminateCheckBox.OnStateChangedListener listener;

        HideAppInfo(ApplicationInfo appInfo, Set<String> set) {
            info = appInfo;
            name = Utils.getAppLabel(info, pm);
            expanded = false;
            processList = StreamSupport.stream(set)
                    .map(process -> new HideProcessInfo(info.packageName, process))
                    .sorted().collect(Collectors.toList());
            listener = (IndeterminateCheckBox box, @Nullable Boolean status) -> {
                if (status != null) {
                    for (HideProcessInfo p : processList) {
                        String cmd = Utils.fmt("magiskhide --%s %s %s",
                                status ? "add" : "rm", info.packageName, p.name);
                        Shell.su(cmd).submit();
                        p.hidden = status;
                    }
                }
            };
        }

        @Override
        public int compareTo(HideAppInfo o) {
            Comparator<HideAppInfo> c;
            c = Comparators.comparing(HideAppInfo::haveHidden);
            c = Comparators.reversed(c);
            c = Comparators.thenComparing(c, t -> t.name, String::compareToIgnoreCase);
            c = Comparators.thenComparing(c, t -> t.info.packageName);
            return c.compare(this, o);
        }

        Boolean getState() {
            boolean all = true;
            boolean hidden = false;
            for (HideProcessInfo p : processList) {
                if (!p.hidden)
                    all = false;
                else
                    hidden = true;
            }
            if (all)
                return true;
            return hidden ? null : false;
        }

        boolean haveHidden() {
            Boolean c = getState();
            return c == null ? true : c;
        }
    }

    class HideProcessInfo implements Comparable<HideProcessInfo> {
        String name;
        boolean hidden;

        HideProcessInfo(String pkg, String process) {
            this.name = process;
            for (HideTarget t : hideList) {
                if (t.pkg.equals(pkg) && t.process.equals(process)) {
                    hidden = true;
                    break;
                }
            }
        }

        @Override
        public int compareTo(HideProcessInfo o) {
            Comparator<HideProcessInfo> c;
            c = Comparators.comparing((HideProcessInfo t) -> t.hidden);
            c = Comparators.reversed(c);
            c = Comparators.thenComparing(c, t -> t.name);
            return c.compare(this, o);
        }
    }

    class HideTarget {
        String pkg;
        String process;

        HideTarget(String line) {
            String[] split = line.split("\\|", 2);
            pkg = split[0];
            if (split.length == 2) {
                process = split[1];
            } else {
                // Backwards compatibility
                old_hide = true;
                process = pkg;
            }
        }
    }

    class AppViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.app_icon) ImageView app_icon;
        @BindView(R.id.app_name) TextView app_name;
        @BindView(R.id.package_name) TextView package_name;
        @BindView(R.id.checkbox) IndeterminateCheckBox checkBox;
        @BindView(R.id.trigger) View trigger;
        @BindView(R.id.arrow) ImageView arrow;

        Expandable ex;

        AppViewHolder(@NonNull View itemView) {
            super(itemView);
            new ApplicationAdapter$AppViewHolder_ViewBinding(this, itemView);
            ex = new ArrowExpandable(new Expandable() {
                @Override
                protected void onExpand() {
                    getMargins(AppViewHolder.this).bottomMargin = 0;
                }

                @Override
                protected void onCollapse() {
                    getMargins(AppViewHolder.this).bottomMargin = BOTTOM_MARGIN;
                }
            }, arrow);
        }
    }

    class ProcessViewHolder extends RecyclerView.ViewHolder {

        @BindView(R.id.process) TextView process;
        @BindView(R.id.checkbox) CheckBox checkbox;

        ProcessViewHolder(@NonNull View itemView) {
            super(itemView);
            new ApplicationAdapter$ProcessViewHolder_ViewBinding(this, itemView);
        }
    }

}
