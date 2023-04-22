package com.topjohnwu.magisk.ui.home

import android.os.Bundle
import android.view.LayoutInflater
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import androidx.core.view.MenuProvider
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseFragment
import com.topjohnwu.magisk.arch.viewModel
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.download.DownloadService
import com.topjohnwu.magisk.databinding.FragmentHomeMd2Binding

class HomeFragment : BaseFragment<FragmentHomeMd2Binding>(), MenuProvider {

    override val layoutRes = R.layout.fragment_home_md2
    override val viewModel by viewModel<HomeViewModel>()

    override fun onStart() {
        super.onStart()
        activity?.setTitle(R.string.section_home)
        DownloadService.observeProgress(this, viewModel::onProgressUpdate)
    }

    private fun checkTitle(text: TextView, icon: ImageView) {
        text.post {
            if (text.layout?.getEllipsisCount(0) != 0) {
                with (icon) {
                    layoutParams.width = 0
                    layoutParams.height = 0
                    requestLayout()
                }
            }
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        super.onCreateView(inflater, container, savedInstanceState)

        // If titles are squished, hide icons
        with(binding.homeMagiskWrapper) {
            checkTitle(homeMagiskTitle, homeMagiskIcon)
        }
        with(binding.homeManagerWrapper) {
            checkTitle(homeManagerTitle, homeManagerIcon)
        }

        return binding.root
    }

    override fun onCreateMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_home_md2, menu)
        if (!Info.isRooted)
            menu.removeItem(R.id.action_reboot)
    }

    override fun onMenuItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_settings ->
                HomeFragmentDirections.actionHomeFragmentToSettingsFragment().navigate()
            R.id.action_reboot -> activity?.let { RebootMenu.inflate(it).show() }
            else -> return super.onOptionsItemSelected(item)
        }
        return true
    }

    override fun onResume() {
        super.onResume()
        viewModel.stateManagerProgress = 0
    }
}
