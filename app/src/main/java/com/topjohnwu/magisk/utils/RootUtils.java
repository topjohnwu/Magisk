package com.topjohnwu.magisk.utils;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;

import androidx.annotation.NonNull;

import com.topjohnwu.magisk.Config;
import com.topjohnwu.magisk.Const;
import com.topjohnwu.magisk.R;
import com.topjohnwu.superuser.Shell;
import com.topjohnwu.superuser.ShellUtils;
import com.topjohnwu.superuser.io.SuFile;

import java.io.InputStream;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.List;

public class RootUtils extends Shell.Initializer {

    public static void rmAndLaunch(String rm, ComponentName component) {
        Shell.su(Utils.fmt("(rm_launch %s %s)&", rm, component.flattenToString())).exec();
    }

    public static void reboot() {
        Shell.su("/system/bin/reboot" + (Config.recovery ? " recovery" : "")).submit();
    }

    public static void startActivity(Intent intent) {
        if (intent.getComponent() == null)
            return;
        List<String> args = new ArrayList<>();
        args.add("am");
        args.add("start");
        intentToCommand(intent, args);
        Shell.su(Utils.argsToCommand(args)).exec();
    }

    public static void intentToCommand(Intent intent, List<String> args) {
        if (intent.getAction() != null) {
            args.add("-a");
            args.add(intent.getAction());
        }
        if (intent.getComponent() != null) {
            args.add("-n");
            args.add(intent.getComponent().flattenToString());
        }
        if (intent.getData() != null) {
            args.add("-d");
            args.add(intent.getDataString());
        }
        if (intent.getCategories() != null) {
            for (String cat : intent.getCategories()) {
                args.add("-c");
                args.add(cat);
            }
        }
        if (intent.getType() != null) {
            args.add("-t");
            args.add(intent.getType());
        }
        Bundle extras = intent.getExtras();
        if (extras != null) {
            for (String key : extras.keySet()) {
                Object val = extras.get(key);
                if (val == null)
                    continue;
                Object value = val;
                String arg;
                if (val instanceof String)          arg = "--es";
                else if (val instanceof Boolean)    arg = "--ez";
                else if (val instanceof Integer)    arg = "--ei";
                else if (val instanceof Long)       arg = "--el";
                else if (val instanceof Float)      arg = "--ef";
                else if (val instanceof Uri)        arg = "--eu";
                else if (val instanceof ComponentName) {
                    arg = "--ecn";
                    value = ((ComponentName) val).flattenToString();
                } else if (val instanceof ArrayList) {
                    ArrayList arr = (ArrayList) val;
                    if (arr.size() <= 0)
                    /* Impossible to know the type due to type erasure */
                        continue;

                    if (arr.get(0) instanceof Integer)      arg = "--eial";
                    else if (arr.get(0) instanceof Long)    arg = "--elal";
                    else if (arr.get(0) instanceof Float)   arg = "--efal";
                    else if (arr.get(0) instanceof String)  arg = "--esal";
                    else                                    continue;  /* Unsupported */
                    StringBuilder sb = new StringBuilder();
                    for (Object o : arr) {
                        sb.append(o.toString().replace(",", "\\,"));
                        sb.append(',');
                    }
                    // Remove trailing comma
                    sb.deleteCharAt(sb.length() - 1);
                    value = sb;
                } else if (val.getClass().isArray()) {
                    if (val instanceof int[])           arg = "--eia";
                    else if (val instanceof long[])     arg = "--ela";
                    else if (val instanceof float[])    arg = "--efa";
                    else if (val instanceof String[])   arg = "--esa";
                    else                                continue;  /* Unsupported */
                    StringBuilder sb = new StringBuilder();
                    int len = Array.getLength(val);
                    for (int i = 0; i < len; ++i) {
                        sb.append(Array.get(val, i).toString().replace(",", "\\,"));
                        sb.append(',');
                    }
                    // Remove trailing comma
                    sb.deleteCharAt(sb.length() - 1);
                    value = sb;
                }
                else continue;  /* Unsupported */

                args.add(arg);
                args.add(key);
                args.add(value.toString());
            }
        }
        args.add("-f");
        args.add(String.valueOf(intent.getFlags()));
    }

    @Override
    public boolean onInit(Context context, @NonNull Shell shell) {
        Shell.Job job = shell.newJob();
        if (shell.isRoot()) {
            job.add(context.getResources().openRawResource(R.raw.util_functions))
                .add(context.getResources().openRawResource(R.raw.utils));
            Const.MAGISK_DISABLE_FILE = new SuFile("/cache/.disable_magisk");
            Config.loadMagiskInfo();
        } else {
            InputStream nonroot = context.getResources().openRawResource(R.raw.nonroot_utils);
            job.add(nonroot);
        }

        job.add("mount_partitions", "get_flags", "run_migrations", "export BOOTMODE=true").exec();

        Config.keepVerity = Boolean.parseBoolean(ShellUtils.fastCmd("echo $KEEPVERITY"));
        Config.keepEnc = Boolean.parseBoolean(ShellUtils.fastCmd("echo $KEEPFORCEENCRYPT"));
        Config.recovery = Boolean.parseBoolean(ShellUtils.fastCmd("echo $RECOVERYMODE"));
        return true;
    }
}
