package com.topjohnwu.magisk.redesign.module

import android.content.Intent
import android.graphics.Insets
import android.os.Bundle
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import androidx.core.view.isVisible
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentModuleMd2Binding
import com.topjohnwu.magisk.model.events.InstallExternalModuleEvent
import com.topjohnwu.magisk.redesign.MainActivity
import com.topjohnwu.magisk.redesign.ReselectionTarget
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import com.topjohnwu.magisk.redesign.compat.hideKeyboard
import com.topjohnwu.magisk.utils.EndlessRecyclerScrollListener
import com.topjohnwu.magisk.utils.MotionRevealHelper
import com.topjohnwu.magisk.utils.PinchZoomTouchListener
import org.koin.androidx.viewmodel.ext.android.viewModel

class ModuleFragment : CompatFragment<ModuleViewModel, FragmentModuleMd2Binding>(),
    ReselectionTarget {

    override val layoutRes = R.layout.fragment_module_md2
    override val viewModel by viewModel<ModuleViewModel>()

    private val listeners = hashSetOf<EndlessRecyclerScrollListener>()

    private var isFilterVisible
        get() = binding.moduleFilter.isVisible
        set(value) {
            if (!value) hideKeyboard()
            (activity as? MainActivity)?.requestNavigationHidden(value)
            MotionRevealHelper.withViews(binding.moduleFilter, binding.moduleFilterToggle, value)
        }

    override fun consumeSystemWindowInsets(insets: Insets) = insets

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        InstallExternalModuleEvent.onActivityResult(requireContext(), requestCode, resultCode, data)
    }

    override fun onStart() {
        super.onStart()
        setHasOptionsMenu(true)
        activity.title = resources.getString(R.string.section_modules)
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

        PinchZoomTouchListener.attachTo(binding.moduleFilterInclude.moduleFilterList)
        PinchZoomTouchListener.attachTo(binding.moduleList)
    }

    override fun onDestroyView() {
        listeners.forEach {
            binding.moduleList.removeOnScrollListener(it)
            binding.moduleFilterInclude.moduleFilterList.removeOnScrollListener(it)
        }
        PinchZoomTouchListener.clear(binding.moduleList)
        PinchZoomTouchListener.clear(binding.moduleFilterInclude.moduleFilterList)
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

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_module_md2, menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_refresh -> viewModel.loadRemoteImplicit()
        }
        return super.onOptionsItemSelected(item)
    }

    // ---

    override fun onReselected() {
        binding.moduleList
            .takeIf {
                (it.layoutManager as? StaggeredGridLayoutManager)?.let {
                    it.findFirstVisibleItemPositions(IntArray(it.spanCount)).min()
                } ?: 0 > 10
            }
            ?.also { it.scrollToPosition(10) }
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