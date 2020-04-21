package com.topjohnwu.magisk.ui.home

import android.os.Bundle
import android.view.*
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentHomeMd2Binding
import com.topjohnwu.magisk.model.events.RebootEvent
import com.topjohnwu.magisk.ui.base.BaseUIFragment
import com.topjohnwu.superuser.Shell
import org.koin.androidx.viewmodel.ext.android.viewModel

class HomeFragment : BaseUIFragment<HomeViewModel, FragmentHomeMd2Binding>() {

    override val layoutRes = R.layout.fragment_home_md2
    override val viewModel by viewModel<HomeViewModel>()

    override fun onStart() {
        super.onStart()
        activity.title = resources.getString(R.string.section_home)
        setHasOptionsMenu(true)
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        super.onCreateView(inflater, container, savedInstanceState)

        // Set barrier reference IDs in code, since resource IDs will be stripped in release mode
        binding.homeMagiskWrapper.homeMagiskTitleBarrier.referencedIds =
            intArrayOf(R.id.home_magisk_action, R.id.home_magisk_title, R.id.home_magisk_icon)
        binding.homeMagiskWrapper.homeMagiskBarrier.referencedIds =
            intArrayOf(R.id.home_magisk_latest_version, R.id.home_magisk_installed_version, R.id.home_magisk_mode)
        binding.homeManagerWrapper.homeManagerTitleBarrier.referencedIds =
            intArrayOf(R.id.home_manager_action, R.id.home_manager_title, R.id.home_manager_icon)

        return binding.root
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_home_md2, menu)
        if (!Shell.rootAccess())
            menu.removeItem(R.id.action_reboot)
    }

    override fun onOptionsItemSelected(item: MenuItem) = when (item.itemId) {
        R.id.action_settings -> HomeFragmentDirections.actionHomeFragmentToSettingsFragment()
            .navigate()
        R.id.action_reboot -> RebootEvent.inflateMenu(activity).show()
        else -> null
    }?.let { true } ?: super.onOptionsItemSelected(item)

}
