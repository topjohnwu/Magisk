package com.topjohnwu.magisk.arch

import androidx.annotation.MainThread
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch

abstract class AsyncLoadViewModel : BaseViewModel() {

    private var loadingJob: Job? = null

    @MainThread
    fun startLoading() {
        if (loadingJob?.isActive == true) {
            return
        }
        loadingJob = viewModelScope.launch { doLoadWork() }
    }

    @MainThread
    fun reload() {
        loadingJob?.cancel()
        loadingJob = viewModelScope.launch { doLoadWork() }
    }

    protected abstract suspend fun doLoadWork()
}
