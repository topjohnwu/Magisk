package com.topjohnwu.magisk.ui.hide

import android.content.Context
import android.os.Bundle
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import androidx.core.view.isVisible
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentHideMd2Binding
import com.topjohnwu.magisk.extensions.hideKeyboard
import com.topjohnwu.magisk.ui.base.BaseUIFragment
import com.topjohnwu.magisk.utils.MotionRevealHelper
import org.koin.androidx.viewmodel.ext.android.viewModel

class HideFragment : BaseUIFragment<HideViewModel, FragmentHideMd2Binding>() {

    override val layoutRes = R.layout.fragment_hide_md2
    override val viewModel by viewModel<HideViewModel>()

    private var isFilterVisible
        get() = binding.hideFilter.isVisible
        set(value) {
            if (!value) hideKeyboard()
            MotionRevealHelper.withViews(binding.hideFilter, binding.hideFilterToggle, value)
        }

    override fun onAttach(context: Context) {
        super.onAttach(context)
        activity.setTitle(R.string.magiskhide)
        setHasOptionsMenu(true)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        binding.hideFilterToggle.setOnClickListener {
            isFilterVisible = true
        }
        binding.hideFilterInclude.hideFilterDone.setOnClickListener {
            isFilterVisible = false
        }
        binding.hideContent.addOnScrollListener(object : RecyclerView.OnScrollListener() {
            override fun onScrollStateChanged(recyclerView: RecyclerView, newState: Int) {
                if (newState != RecyclerView.SCROLL_STATE_IDLE) hideKeyboard()
            }
        })

        val lama = binding.hideContent.layoutManager ?: return
        lama.isAutoMeasureEnabled = false
    }

    override fun onPreBind(binding: FragmentHideMd2Binding) = Unit

    override fun onBackPressed(): Boolean {
        if (isFilterVisible) {
            isFilterVisible = false
            return true
        }
        return super.onBackPressed()
    }

    // ---

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_hide_md2, menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_focus_up -> binding.hideContent
                .takeIf { (it.layoutManager as? LinearLayoutManager)?.findFirstVisibleItemPosition() ?: 0 > 10 }
                ?.also { it.scrollToPosition(10) }
                .let { binding.hideContent }
                .also { it.post { it.smoothScrollToPosition(0) } }
        }
        return super.onOptionsItemSelected(item)
    }

}
