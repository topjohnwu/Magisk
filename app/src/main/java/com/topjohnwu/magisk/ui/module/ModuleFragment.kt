package com.topjohnwu.magisk.ui.module

import android.os.Bundle
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import androidx.core.view.isVisible
import androidx.recyclerview.widget.RecyclerView
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseUIFragment
import com.topjohnwu.magisk.arch.ReselectionTarget
import com.topjohnwu.magisk.arch.ViewEvent
import com.topjohnwu.magisk.core.download.BaseDownloader
import com.topjohnwu.magisk.databinding.FragmentModuleMd2Binding
import com.topjohnwu.magisk.di.viewModel
import com.topjohnwu.magisk.ktx.*
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.utils.EndlessRecyclerScrollListener
import com.topjohnwu.magisk.utils.MotionRevealHelper

class ModuleFragment : BaseUIFragment<ModuleViewModel, FragmentModuleMd2Binding>(),
    ReselectionTarget {

    override val layoutRes = R.layout.fragment_module_md2
    override val viewModel by viewModel<ModuleViewModel>()

    private val listeners = hashSetOf<EndlessRecyclerScrollListener>()

    private var isFilterVisible
        get() = binding.moduleFilter.isVisible
        set(value) {
            if (!value) hideKeyboard()
            MotionRevealHelper.withViews(binding.moduleFilter, binding.moduleFilterToggle, value)
            with(activity as MainActivity) {
                requestNavigationHidden(value)
                setDisplayHomeAsUpEnabled(value)
            }
        }

    override fun onStart() {
        super.onStart()
        setHasOptionsMenu(true)
        activity.title = resources.getString(R.string.modules)
        BaseDownloader.observeProgress(this, viewModel::onProgressUpdate)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        setEndlessScroller()
        setEndlessSearch()

        binding.moduleFilterToggle.setOnClickListener {
            isFilterVisible = true
        }
        binding.moduleFilterInclude.moduleFilterDone.setOnClickListener {
            isFilterVisible = false
        }
        binding.moduleFilterInclude.moduleFilterList.addOnScrollListener(object :
            RecyclerView.OnScrollListener() {
            override fun onScrollStateChanged(recyclerView: RecyclerView, newState: Int) {
                if (newState != RecyclerView.SCROLL_STATE_IDLE) hideKeyboard()
            }
        })

        val resource = requireContext().resources
        val l_50 = resource.getDimensionPixelSize(R.dimen.l_50)
        val l1 = resource.getDimensionPixelSize(R.dimen.l1)
        binding.moduleList.apply {
            addVerticalPadding(
                l_50,
                l1 + l_50 + resource.getDimensionPixelSize(R.dimen.internal_action_bar_size)
            )
            addSimpleItemDecoration(
                left = l1,
                top = l_50,
                right = l1,
                bottom = l_50,
            )
            fixEdgeEffect()
            post {
                addInvalidateItemDecorationsObserver()
            }
        }

        binding.moduleFilterInclude.moduleFilterList.apply {
            addVerticalPadding(
                l_50,
                l_50
            )
            addSimpleItemDecoration(
                left = l1,
                top = l_50,
                right = l1,
                bottom = l_50,
            )
            fixEdgeEffect()
        }
    }

    override fun onDestroyView() {
        listeners.forEach {
            binding.moduleList.removeOnScrollListener(it)
            binding.moduleFilterInclude.moduleFilterList.removeOnScrollListener(it)
        }
        super.onDestroyView()
    }

    override fun onBackPressed(): Boolean {
        if (isFilterVisible) {
            isFilterVisible = false
            return true
        }
        return super.onBackPressed()
    }

    // ---

    override fun onEventDispatched(event: ViewEvent) = when (event) {
        is EndlessRecyclerScrollListener.ResetState -> listeners.forEach { it.resetState() }
        else -> super.onEventDispatched(event)
    }

    // ---

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_module_md2, menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_refresh -> viewModel.forceRefresh()
        }
        return super.onOptionsItemSelected(item)
    }

    // ---

    override fun onReselected() {
        binding.moduleList
            .also { it.scrollToPosition(10) }
            .let { binding.moduleList }
            .also { it.post { it.smoothScrollToPosition(0) } }
    }

    // ---

    override fun onPreBind(binding: FragmentModuleMd2Binding) = Unit

    private fun setEndlessScroller() {
        val lama = binding.moduleList.layoutManager ?: return
        lama.isAutoMeasureEnabled = false

        val listener = EndlessRecyclerScrollListener(lama, viewModel::loadRemote)
        binding.moduleList.addOnScrollListener(listener)
        listeners.add(listener)
    }

    private fun setEndlessSearch() {
        val lama = binding.moduleFilterInclude.moduleFilterList.layoutManager ?: return
        lama.isAutoMeasureEnabled = false

        val listener = EndlessRecyclerScrollListener(lama, viewModel::loadMoreQuery)
        binding.moduleFilterInclude.moduleFilterList.addOnScrollListener(listener)
        listeners.add(listener)
    }

}
