package com.topjohnwu.magisk.redesign.module

import android.graphics.Insets
import android.os.Bundle
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentModuleMd2Binding
import com.topjohnwu.magisk.redesign.MainActivity
import com.topjohnwu.magisk.redesign.ReselectionTarget
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import com.topjohnwu.magisk.redesign.compat.hideKeyboard
import com.topjohnwu.magisk.redesign.hide.MotionRevealHelper
import com.topjohnwu.magisk.utils.EndlessRecyclerScrollListener
import org.koin.androidx.viewmodel.ext.android.viewModel

class ModuleFragment : CompatFragment<ModuleViewModel, FragmentModuleMd2Binding>(),
    ReselectionTarget {

    override val layoutRes = R.layout.fragment_module_md2
    override val viewModel by viewModel<ModuleViewModel>()

    private val listeners = hashSetOf<EndlessRecyclerScrollListener>()

    override fun consumeSystemWindowInsets(insets: Insets) = insets

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
            (activity as? MainActivity)?.requestNavigationHidden()
            MotionRevealHelper.withViews(binding.moduleFilter, binding.moduleFilterToggle, true)
        }
        binding.moduleFilterInclude.moduleFilterDone.setOnClickListener {
            (activity as? MainActivity)?.requestNavigationHidden(false)
            hideKeyboard()
            MotionRevealHelper.withViews(binding.moduleFilter, binding.moduleFilterToggle, false)
        }
    }

    override fun onDestroyView() {
        listeners.forEach {
            binding.moduleRemote.removeOnScrollListener(it)
            binding.moduleFilterInclude.moduleFilterList.removeOnScrollListener(it)
        }
        super.onDestroyView()
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
        binding.moduleRemote
            .takeIf {
                (it.layoutManager as? StaggeredGridLayoutManager)?.let {
                    it.findFirstVisibleItemPositions(IntArray(it.spanCount)).min()
                } ?: 0 > 10
            }
            ?.also { it.scrollToPosition(10) }
            .let { binding.moduleRemote }
            .also { it.post { it.smoothScrollToPosition(0) } }
    }

    // ---

    override fun onPreBind(binding: FragmentModuleMd2Binding) = Unit

    private fun setEndlessScroller() {
        val lama = binding.moduleRemote.layoutManager ?: return
        lama.isAutoMeasureEnabled = false

        val listener = EndlessRecyclerScrollListener(lama, viewModel::loadRemote)
        binding.moduleRemote.addOnScrollListener(listener)
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