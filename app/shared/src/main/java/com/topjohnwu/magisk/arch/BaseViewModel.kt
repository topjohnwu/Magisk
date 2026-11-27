package com.topjohnwu.magisk.arch

import android.widget.Toast
import androidx.annotation.StringRes
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.R
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.core.utils.MediaStoreUtils
import com.topjohnwu.magisk.core.utils.MediaStoreUtils.outputStream
import com.topjohnwu.magisk.ui.navigation.Route
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.receiveAsFlow
import kotlinx.coroutines.launch
import timber.log.Timber
import java.io.Writer

abstract class BaseViewModel : ViewModel() {

    // Keep requests until the UI collects them: one can arrive before the screen's collector
    // has (re)started, e.g. from an activity result delivered right after a recreation
    private val _navEvents = Channel<Route>(Channel.BUFFERED)
    val navEvents: Flow<Route> = _navEvents.receiveAsFlow()

    fun showToast(@StringRes resId: Int) {
        AppContext.toast(resId, Toast.LENGTH_SHORT)
    }

    fun showToast(msg: String) {
        AppContext.toast(msg, Toast.LENGTH_SHORT)
    }

    fun navigateTo(route: Route) {
        _navEvents.trySend(route)
    }

    /**
     * Save a log to the download folder with [write], then show where it was saved. Saving can
     * fail for many reasons (e.g. storage full, or the media store rejecting the file), so
     * report failures instead of letting them crash the app.
     */
    protected fun saveLogFile(name: String, write: (Writer) -> Unit) {
        viewModelScope.launch(Dispatchers.IO) {
            try {
                // Names can contain parts like module names, which must not create directories
                val file = MediaStoreUtils.getFile(name.replace('/', '_'))
                file.uri.outputStream().bufferedWriter().use(write)
                showToast(file.toString())
            } catch (e: CancellationException) {
                throw e
            } catch (e: Exception) {
                Timber.e(e)
                showToast(R.string.log_save_failed)
            }
        }
    }
}
