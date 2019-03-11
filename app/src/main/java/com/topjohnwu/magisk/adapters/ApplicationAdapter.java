package com.topjohnwu.magisk.adapters;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.ComponentInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.AsyncTask;

import androidx.annotation.WorkerThread;
import androidx.collection.ArraySet;

import com.topjohnwu.magisk.App;
import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.utils.Topic;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.internal.UiThreadHandler;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import java9.util.stream.Collectors;
import java9.util.stream.Stream;
import java9.util.stream.StreamSupport;
import me.drakeet.multitype.MultiTypeAdapter;

import static com.topjohnwu.magisk.utils.Utils.getAppLabel;


public class ApplicationAdapter {

    /* A list of apps that should not be shown as hide-able */
    private static final List<String> HIDE_BLACKLIST = Arrays.asList(
            App.self.getPackageName(),
            "android",
            "com.android.chrome",
            "com.chrome.beta",
            "com.chrome.dev",
            "com.chrome.canary",
            "com.android.webview",
            "com.google.android.webview"
    );
    private static final String SAFETYNET_PROCESS = "com.google.android.gms.unstable";
    private static final String GMS_PACKAGE = "com.google.android.gms";

    private List<AppViewBinder.App> fullList;
    private List<HideTarget> hideList;
    private List<Object> showList;
    private MultiTypeAdapter adapter;
    private PackageManager pm;
    private boolean showSystem;

    public ApplicationAdapter(Context context, MultiTypeAdapter adapter) {
        hideList = Collections.emptyList();
        fullList = new ArrayList<>();
        showList = new ArrayList<>();
        this.adapter = adapter;
        this.adapter.setItems(showList);
        pm = context.getPackageManager();
        showSystem = Config.get(Config.Key.SHOW_SYSTEM_APP);
        AsyncTask.SERIAL_EXECUTOR.execute(this::loadApps);
        adapter.register(AppViewBinder.App.class, new AppViewBinder(showList));
        adapter.register(ProcessViewBinder.Process.class, new ProcessViewBinder());
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
        List<ApplicationInfo> installed = pm.getInstalledApplications(0);

        hideList = StreamSupport.stream(Shell.su("magiskhide --ls").exec().getOut())
                .map(HideTarget::new).collect(Collectors.toList());

        fullList.clear();

        for (ApplicationInfo info : installed) {
            // Do not show black-listed and disabled apps
            if (!HIDE_BLACKLIST.contains(info.packageName) && info.enabled) {
                AppViewBinder.App app = new AppViewBinder.App(info.loadIcon(pm),
                        getAppLabel(info, pm), info.packageName, info.flags);
                fullList.add(app);

                Set<String> set = new ArraySet<>();
                PackageInfo pkg = getPackageInfo(info.packageName);
                if (pkg != null) {
                    addProcesses(set, pkg.activities);
                    addProcesses(set, pkg.services);
                    addProcesses(set, pkg.receivers);
                    addProcesses(set, pkg.providers);
                } else {
                    set.add(info.packageName);
                }
                if (set.isEmpty()) fullList.remove(app);
                for (String proc : set) {
                    boolean hidden = false;
                    for (HideTarget tgt : hideList) {
                        if (info.packageName.equals(tgt.pkg) && proc.equals(tgt.process))
                            hidden = true;
                    }
                    app.processes.add(new ProcessViewBinder.Process(proc, hidden, info.packageName));
                }
                app.getStat(true);
                Collections.sort(app.processes);
            }
        }

        Collections.sort(fullList);
        Topic.publish(false, Topic.MAGISK_HIDE_DONE);
    }

    public void setShowSystem(boolean b) {
        showSystem = b;
    }


    // True if not system app and have launch intent, or user already hidden it
    private boolean systemFilter(AppViewBinder.App target) {
        return showSystem || target.stat != 0 ||
                ((target.flags & ApplicationInfo.FLAG_SYSTEM) == 0 &&
                        pm.getLaunchIntentForPackage(target.packageName) != null);
    }

    private boolean contains(String s, String filter) {
        return s.toLowerCase().contains(filter);
    }

    private boolean nameFilter(AppViewBinder.App target, String filter) {
        if (filter == null || filter.isEmpty())
            return true;
        filter = filter.toLowerCase();
        return contains(target.name, filter) ||
                contains(target.packageName, filter);
    }

    public void filter(String constraint) {
        AsyncTask.SERIAL_EXECUTOR.execute(() -> {
            Stream<AppViewBinder.App> s = StreamSupport.stream(fullList)
                    .filter(this::systemFilter)
                    .filter(t -> nameFilter(t, constraint));
            UiThreadHandler.run(() -> {
                showList.clear();
                for (AppViewBinder.App target : s.collect(Collectors.toList())) {
                    if (target.expand) {
                        showList.add(target);
                        showList.addAll(target.processes);
                    } else showList.add(target);
                }
                adapter.notifyDataSetChanged();
            });
        });
    }

    public void refresh() {
        Collections.sort(fullList);
        Topic.publish(false, Topic.MAGISK_HIDE_DONE);
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
