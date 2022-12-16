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
            // Prevent multiple jobs from running at the same time
            return
        }
        loadingJob = viewModelScope.launch { doLoadWork() }
    }

    protected abstract suspend fun doLoadWork()
}
