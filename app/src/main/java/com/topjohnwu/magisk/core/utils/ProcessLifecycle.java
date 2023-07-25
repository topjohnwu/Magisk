package com.topjohnwu.magisk.core.utils;

import android.content.Context;

import androidx.annotation.NonNull;
import androidx.lifecycle.LifecycleDispatcher;
import androidx.lifecycle.ProcessLifecycleOwner;

// Use Java to bypass Kotlin internal visibility modifier
public class ProcessLifecycle {
    public static void init(@NonNull Context context) {
        LifecycleDispatcher.init(context);
        ProcessLifecycleOwner.init$lifecycle_process_release(context);
    }
}
