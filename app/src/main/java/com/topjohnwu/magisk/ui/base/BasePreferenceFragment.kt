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
import com.topjohnwu.magisk.App
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.KConfig
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.data.database.RepoDatabaseHelper
import com.topjohnwu.magisk.data.repository.SettingRepository
import org.koin.android.ext.android.inject

abstract class BasePreferenceFragment : PreferenceFragmentCompat(),
    SharedPreferences.OnSharedPreferenceChangeListener {

    protected val repoDatabase: RepoDatabaseHelper by inject()
    protected val prefs: SharedPreferences by inject()
    protected val app: App by inject()
    protected val settingRepo: SettingRepository by inject()

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

    protected fun setCustomUpdateChannel(userRepo: String) {
        KConfig.customUpdateChannel = userRepo
    }

    protected fun getChannelCompat(channel: Int): KConfig.UpdateChannel {
        return when (channel) {
            Config.Value.STABLE_CHANNEL,
            Config.Value.DEFAULT_CHANNEL -> KConfig.UpdateChannel.STABLE
            Config.Value.BETA_CHANNEL -> KConfig.UpdateChannel.BETA
            Config.Value.CANARY_CHANNEL -> KConfig.UpdateChannel.CANARY
            Config.Value.CANARY_DEBUG_CHANNEL -> KConfig.UpdateChannel.CANARY_DEBUG
            Config.Value.CUSTOM_CHANNEL -> KConfig.UpdateChannel.CUSTOM
            else -> KConfig.updateChannel
        }
    }
}
