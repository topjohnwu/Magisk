package com.topjohnwu.magisk.ui.home

import android.os.Bundle
import android.view.*
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseUIFragment
import com.topjohnwu.magisk.core.download.BaseDownloader
import com.topjohnwu.magisk.databinding.FragmentHomeMd2Binding
import com.topjohnwu.magisk.events.RebootEvent
import com.topjohnwu.superuser.Shell
import org.koin.androidx.viewmodel.ext.android.viewModel

class HomeFragment : BaseUIFragment<HomeViewModel, FragmentHomeMd2Binding>() {

    override val layoutRes = R.layout.fragment_home_md2
    override val viewModel by viewModel<HomeViewModel>()

    override fun onStart() {
        super.onStart()
        activity.title = resources.getString(R.string.section_home)
        setHasOptionsMenu(true)
        BaseDownloader.observeProgress(this, viewModel::onProgressUpdate)
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        super.onCreateView(inflater, container, savedInstanceState)

        // If titles are squished, hide icons

        with(binding.homeMagiskWrapper) {
            with(homeMagiskTitle) {
                post {
                    if (lineCount != 1) {
                        with(homeMagiskIcon) {
                            layoutParams.width = 0
                            layoutParams.height = 0
                            requestLayout()
                        }
                    }
                }
            }
        }

        with(binding.homeManagerWrapper) {
            with(homeManagerTitle) {
                post {
                    if (lineCount != 1) {
                        with(homeManagerIcon) {
                            layoutParams.width = 0
                            layoutParams.height = 0
                            requestLayout()
                        }
                    }
                }
            }
        }

        return binding.root
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_home_md2, menu)
        if (!Shell.rootAccess())
            menu.removeItem(R.id.action_reboot)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_settings ->
                HomeFragmentDirections.actionHomeFragmentToSettingsFragment().navigate()
            R.id.action_reboot -> RebootEvent.inflateMenu(activity).show()
            else -> return super.onOptionsItemSelected(item)
        }
        return true
    }

}
