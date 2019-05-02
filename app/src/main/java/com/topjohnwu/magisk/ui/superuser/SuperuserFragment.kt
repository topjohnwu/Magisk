package com.topjohnwu.magisk.ui.superuser

import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentSuperuserBinding
import com.topjohnwu.magisk.ui.base.MagiskFragment
import org.koin.androidx.viewmodel.ext.android.viewModel

class SuperuserFragment :
    MagiskFragment<SuperuserViewModel, FragmentSuperuserBinding>() {

    override val layoutRes: Int = R.layout.fragment_superuser
    override val viewModel: SuperuserViewModel by viewModel()

    override fun onStart() {
        super.onStart()
        setHasOptionsMenu(true)
        requireActivity().setTitle(R.string.superuser)
    }

    override fun onResume() {
        super.onResume()
        viewModel.updatePolicies()
    }

}
