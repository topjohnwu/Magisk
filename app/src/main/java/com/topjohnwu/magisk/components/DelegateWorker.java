package com.topjohnwu.magisk.components;

import android.content.Context;
import android.net.Network;
import android.net.Uri;

import androidx.annotation.MainThread;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.work.Data;
import androidx.work.ListenableWorker;

import com.google.common.util.concurrent.ListenableFuture;

import java.util.List;
import java.util.Set;
import java.util.UUID;

public abstract class DelegateWorker {

    private ListenableWorker worker;

    @NonNull
    public abstract ListenableWorker.Result doWork();

    public void onStopped() {}

    public void setActualWorker(ListenableWorker w) {
        worker = w;
    }

    @NonNull
    public Context getApplicationContext() {
        return worker.getApplicationContext();
    }

    @NonNull
    public UUID getId() {
        return worker.getId();
    }

    @NonNull
    public Data getInputData() {
        return worker.getInputData();
    }

    @NonNull
    public Set<String> getTags() {
        return worker.getTags();
    }

    @NonNull
    @RequiresApi(24)
    public List<Uri> getTriggeredContentUris() {
        return worker.getTriggeredContentUris();
    }

    @NonNull
    @RequiresApi(24)
    public List<String> getTriggeredContentAuthorities() {
        return worker.getTriggeredContentAuthorities();
    }

    @Nullable
    @RequiresApi(28)
    public Network getNetwork() {
        return worker.getNetwork();
    }

    public int getRunAttemptCount() {
        return worker.getRunAttemptCount();
    }

    @NonNull
    @MainThread
    public ListenableFuture<ListenableWorker.Result> startWork() {
        return worker.startWork();
    }

    public boolean isStopped() {
        return worker.isStopped();
    }
}
