package com.topjohnwu.magisk.ui.log

import android.os.Bundle
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import androidx.core.view.isVisible
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentLogMd2Binding
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.base.BaseUIFragment
import com.topjohnwu.magisk.utils.MotionRevealHelper
import org.koin.androidx.viewmodel.ext.android.viewModel

class LogFragment : BaseUIFragment<LogViewModel, FragmentLogMd2Binding>() {

    override val layoutRes = R.layout.fragment_log_md2
    override val viewModel by viewModel<LogViewModel>()

    private var actionSave: MenuItem? = null
    private var isMagiskLogVisible
        get() = binding.logFilter.isVisible
        set(value) {
            MotionRevealHelper.withViews(binding.logFilter, binding.logFilterToggle, value)
            actionSave?.isVisible = !value
            with(activity as MainActivity) {
                invalidateToolbar()
                requestNavigationHidden(value)
                setDisplayHomeAsUpEnabled(value)
            }
        }

    override fun onStart() {
        super.onStart()
        setHasOptionsMenu(true)
        activity.title = resources.getString(R.string.logs)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        binding.logFilterToggle.setOnClickListener {
            isMagiskLogVisible = true
        }
    }


    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        super.onCreateOptionsMenu(menu, inflater)
        inflater.inflate(R.menu.menu_log_md2, menu)
        actionSave = menu.findItem(R.id.action_save)?.also {
            it.isVisible = !isMagiskLogVisible
        }
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_save -> viewModel.saveMagiskLog()
            R.id.action_clear ->
                if (!isMagiskLogVisible) viewModel.clearMagiskLog()
                else viewModel.clearLog()
        }
        return super.onOptionsItemSelected(item)
    }


    override fun onPreBind(binding: FragmentLogMd2Binding) = Unit

    override fun onBackPressed(): Boolean {
        if (binding.logFilter.isVisible) {
            isMagiskLogVisible = false
            return true
        }
        return super.onBackPressed()
    }

}
