package com.topjohnwu.magisk.ui.base

import android.annotation.SuppressLint
import android.content.SharedPreferences
import android.os.Build
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.core.view.children
import androidx.core.view.isVisible
import androidx.preference.*
import androidx.recyclerview.widget.RecyclerView
import com.topjohnwu.magisk.R
import org.koin.android.ext.android.inject

abstract class BasePreferenceFragment : PreferenceFragmentCompat(),
    SharedPreferences.OnSharedPreferenceChangeListener {

    protected val prefs: SharedPreferences by inject()
    protected val activity get() = requireActivity() as MagiskActivity<*, *>

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val v = super.onCreateView(inflater, container, savedInstanceState)
        prefs.registerOnSharedPreferenceChangeListener(this)
        return v
    }

    override fun onDestroyView() {
        prefs.unregisterOnSharedPreferenceChangeListener(this)
        super.onDestroyView()
    }

    override fun onCreateAdapter(preferenceScreen: PreferenceScreen): RecyclerView.Adapter<*> {
        return object : PreferenceGroupAdapter(preferenceScreen) {
            @SuppressLint("RestrictedApi")
            override fun onBindViewHolder(holder: PreferenceViewHolder, position: Int) {
                super.onBindViewHolder(holder, position)
                when (val preference = getItem(position)) {
                    is PreferenceCategory -> setZeroPaddingToLayoutChildren(holder.itemView)
                    else -> holder.itemView.findViewById<View>(R.id.icon_frame)?.isVisible =
                        preference.icon != null
                }
            }
        }
    }

    private fun setZeroPaddingToLayoutChildren(view: View) {
        (view as? ViewGroup)?.children?.forEach {
            setZeroPaddingToLayoutChildren(it)
        } ?: return

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1)
            view.setPaddingRelative(0, view.paddingTop, view.paddingEnd, view.paddingBottom)
        else
            view.setPadding(0, view.paddingTop, view.paddingRight, view.paddingBottom)
    }

    protected fun <T: Preference> findPref(key: CharSequence): T {
        return findPreference(key) as T
    }
}
