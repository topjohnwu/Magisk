package com.topjohnwu.magisk.ui.superuser

import android.os.Bundle
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.databinding.FragmentSuperuserMd2Binding
import com.topjohnwu.magisk.model.navigation.Navigation
import com.topjohnwu.magisk.ui.base.BaseUIFragment
import com.topjohnwu.magisk.utils.PinchZoomTouchListener
import org.koin.androidx.viewmodel.ext.android.viewModel

class SuperuserFragment : BaseUIFragment<SuperuserViewModel, FragmentSuperuserMd2Binding>() {

    override val layoutRes = R.layout.fragment_superuser_md2
    override val viewModel by viewModel<SuperuserViewModel>()

    override fun onStart() {
        super.onStart()
        activity.title = resources.getString(R.string.superuser)
        setHasOptionsMenu(true)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        PinchZoomTouchListener.attachTo(binding.superuserList)
    }

    override fun onDestroyView() {
        PinchZoomTouchListener.clear(binding.superuserList)
        super.onDestroyView()
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_superuser_md2, menu)
    }

    override fun onOptionsItemSelected(item: MenuItem) = when (item.itemId) {
        R.id.action_log -> Navigation.log().dispatchOnSelf()
        else -> null
    }?.let { true } ?: super.onOptionsItemSelected(item)

}
