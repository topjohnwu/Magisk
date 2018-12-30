package com.topjohnwu.magisk.services;

import androidx.annotation.NonNull;
import androidx.work.ListenableWorker;

public abstract class DelegateWorker {
    @NonNull
    public abstract ListenableWorker.Result doWork();
}
