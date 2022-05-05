package com.topjohnwu.magisk.ui.module

import android.os.Bundle
import android.view.View
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseFragment
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.databinding.FragmentModuleMd2Binding
import com.topjohnwu.magisk.databinding.RvItem
import com.topjohnwu.magisk.databinding.adapterOf
import com.topjohnwu.magisk.di.viewModel
import rikka.recyclerview.addEdgeSpacing
import rikka.recyclerview.addInvalidateItemDecorationsObserver
import rikka.recyclerview.addItemSpacing
import rikka.recyclerview.fixEdgeEffect

class ModuleFragment : BaseFragment<FragmentModuleMd2Binding>() {

    override val layoutRes = R.layout.fragment_module_md2
    override val viewModel by viewModel<ModuleViewModel>()

    override fun onStart() {
        super.onStart()
        setHasOptionsMenu(true)
        activity?.title = resources.getString(R.string.modules)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        binding.moduleList.apply {
            adapter = adapterOf<RvItem>()
            addEdgeSpacing(top = R.dimen.l_50, bottom = R.dimen.l1)
            addItemSpacing(R.dimen.l1, R.dimen.l_50, R.dimen.l1)
            fixEdgeEffect()
            post { addInvalidateItemDecorationsObserver() }
        }
    }

    override fun onSaveInstanceState(outState: Bundle) {
        outState.putBoolean(KEY_WAITING_FOR_FILE, viewModel.waitingForFile)
    }

    override fun onViewStateRestored(savedInstanceState: Bundle?) {
        if (savedInstanceState?.getBoolean(KEY_WAITING_FOR_FILE, viewModel.waitingForFile) == true) {
            (requireActivity() as BaseActivity).updateContentCallback(viewModel.selectFileCallback)
        }
        super.onViewStateRestored(savedInstanceState)
    }

    override fun onPreBind(binding: FragmentModuleMd2Binding) = Unit

    companion object {
        private const val KEY_WAITING_FOR_FILE = "waiting_for_file"
    }
}
