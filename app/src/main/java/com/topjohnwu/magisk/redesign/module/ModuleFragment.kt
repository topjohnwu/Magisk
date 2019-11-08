package com.topjohnwu.magisk.redesign.module

import android.graphics.Insets
import android.os.Bundle
import android.view.View
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentModuleMd2Binding
import com.topjohnwu.magisk.redesign.compat.CompatFragment
import com.topjohnwu.magisk.utils.EndlessRecyclerScrollListener
import org.koin.androidx.viewmodel.ext.android.viewModel

class ModuleFragment : CompatFragment<ModuleViewModel, FragmentModuleMd2Binding>() {

    override val layoutRes = R.layout.fragment_module_md2
    override val viewModel by viewModel<ModuleViewModel>()

    private lateinit var listener: EndlessRecyclerScrollListener

    override fun consumeSystemWindowInsets(insets: Insets) = insets

    override fun onStart() {
        super.onStart()
        activity.title = resources.getString(R.string.section_modules)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        setEndlessScroller()
    }

    override fun onDestroyView() {
        if (this::listener.isInitialized) {
            binding.moduleRemote.removeOnScrollListener(listener)
        }
        super.onDestroyView()
    }

    private fun setEndlessScroller() {
        val lama = binding.moduleRemote.layoutManager as? StaggeredGridLayoutManager ?: return
        lama.isAutoMeasureEnabled = false

        listener = EndlessRecyclerScrollListener(lama, viewModel::loadRemote)
        binding.moduleRemote.addOnScrollListener(listener)
    }

}