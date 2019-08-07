package com.topjohnwu.magisk.ui.log


import android.os.Bundle
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import com.skoumal.teanity.viewevents.ViewEvent
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentLogBinding
import com.topjohnwu.magisk.model.events.PageChangedEvent
import com.topjohnwu.magisk.ui.base.MagiskFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class LogFragment : MagiskFragment<LogViewModel, FragmentLogBinding>() {

    override val layoutRes: Int = R.layout.fragment_log
    override val viewModel: LogViewModel by viewModel()

    override fun onEventDispatched(event: ViewEvent) {
        super.onEventDispatched(event)
        when (event) {
            is PageChangedEvent -> activity.invalidateOptionsMenu()
        }
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        binding.logTabs.setupWithViewPager(binding.logContainer, true)
    }

    override fun onStart() {
        super.onStart()
        setHasOptionsMenu(true)
        activity.setTitle(R.string.log)
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_log, menu)
        menu.findItem(R.id.menu_save).isVisible = viewModel.currentPage.value == 1
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.menu_save -> activity.withExternalRW {
                onSuccess {
                    viewModel.saveLog()
                }
            }
            R.id.menu_clear -> viewModel.clearLog()
            R.id.menu_refresh -> viewModel.refresh()
        }
        return true
    }

}
