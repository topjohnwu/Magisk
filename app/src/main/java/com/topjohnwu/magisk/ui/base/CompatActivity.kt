package com.topjohnwu.magisk.ui.base

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.view.View
import android.view.ViewGroup
import android.view.inputmethod.InputMethodManager
import androidx.core.content.getSystemService
import androidx.databinding.OnRebindCallback
import androidx.databinding.ViewDataBinding
import androidx.fragment.app.Fragment
import com.topjohnwu.magisk.extensions.snackbar
import com.topjohnwu.magisk.extensions.startAnimations
import com.topjohnwu.magisk.model.events.SnackbarEvent
import com.topjohnwu.magisk.model.events.ViewEvent
import com.topjohnwu.magisk.model.navigation.Navigator
import com.topjohnwu.magisk.ui.theme.Theme
import kotlin.reflect.KClass

// TODO (diareuse): Merge into BaseUIActivity after all legacy UI is migrated

abstract class CompatActivity<ViewModel : BaseViewModel, Binding : ViewDataBinding> :
    BaseUIActivity<ViewModel, Binding>(), CompatView<ViewModel>, Navigator {

    override val themeRes = Theme.selected.themeRes
    override val viewRoot: View get() = binding.root
    override val navigation: CompatNavigationDelegate<CompatActivity<ViewModel, Binding>>? by lazy {
        CompatNavigationDelegate(this)
    }
    override val baseFragments = listOf<KClass<out Fragment>>()
    private val delegate by lazy { CompatDelegate(this) }

    internal abstract val navHost: Int

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        navigation?.onActivityResult(requestCode, resultCode, data)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding.addOnRebindCallback(object : OnRebindCallback<Binding>() {
            override fun onPreBind(binding: Binding): Boolean {
                (binding.root as? ViewGroup)?.startAnimations()
                return super.onPreBind(binding)
            }
        })

        delegate.onCreate()
        navigation?.onCreate(savedInstanceState)
    }

    override fun onResume() {
        super.onResume()

        delegate.onResume()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        navigation?.onSaveInstanceState(outState)
    }

    override fun onEventDispatched(event: ViewEvent) {
        delegate.onEventExecute(event, this)
        when (event) {
            is SnackbarEvent -> snackbar(snackbarView, event.message(this), event.length, event.f)
        }
    }

    override fun onBackPressed() {
        if (navigation?.onBackPressed()?.not() == true) {
            super.onBackPressed()
        }
    }

    protected fun ViewEvent.dispatchOnSelf() = onEventDispatched(this)

}

fun Activity.hideKeyboard() {
    val view = currentFocus ?: return
    getSystemService<InputMethodManager>()
        ?.hideSoftInputFromWindow(view.windowToken, 0)
    view.clearFocus()
}
