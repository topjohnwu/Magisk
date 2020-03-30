package com.topjohnwu.magisk.ui.superuser

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentSuperuserMd2Binding
import com.topjohnwu.magisk.ui.base.BaseUIFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class SuperuserFragment : BaseUIFragment<SuperuserViewModel, FragmentSuperuserMd2Binding>() {

    override val layoutRes = R.layout.fragment_superuser_md2
    override val viewModel by viewModel<SuperuserViewModel>()

    override fun onStart() {
        super.onStart()
        activity.title = resources.getString(R.string.superuser)
    }

    override fun onPreBind(binding: FragmentSuperuserMd2Binding) {}

}
