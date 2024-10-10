package com.topjohnwu.magisk.ui.module

import android.annotation.SuppressLint
import android.content.pm.ActivityInfo
import android.os.Bundle
import android.view.KeyEvent
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import android.widget.Toast
import androidx.core.view.MenuProvider
import androidx.core.view.isVisible
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.arch.BaseFragment
import com.topjohnwu.magisk.arch.viewModel
import com.topjohnwu.magisk.core.ktx.toast
import com.topjohnwu.magisk.databinding.FragmentActionMd2Binding
import com.topjohnwu.magisk.ui.flash.FlashViewModel
import timber.log.Timber
import com.topjohnwu.magisk.core.R as CoreR

class ActionFragment : BaseFragment<FragmentActionMd2Binding>(), MenuProvider {

    override val layoutRes = R.layout.fragment_action_md2
    override val viewModel by viewModel<ActionViewModel>()
    override val snackbarView: View get() = binding.snackbarContainer

    private var defaultOrientation = -1

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        viewModel.args = ActionFragmentArgs.fromBundle(requireArguments())
    }

    override fun onStart() {
        super.onStart()
        activity?.setTitle(viewModel.args.name)
        binding.closeBtn.setOnClickListener {
            activity?.onBackPressed();
        }

        viewModel.state.observe(this) {
            activity?.supportActionBar?.setSubtitle(
                when (it) {
                    ActionViewModel.State.RUNNING -> CoreR.string.running
                    ActionViewModel.State.SUCCESS -> CoreR.string.done
                    ActionViewModel.State.FAILED -> CoreR.string.failure
                }
            )
            when (it) {
                ActionViewModel.State.SUCCESS -> {
                    activity?.apply {
                        toast(
                            getString(
                                com.topjohnwu.magisk.core.R.string.done_action,
                                this@ActionFragment.viewModel.args.name
                            ), Toast.LENGTH_LONG
                        )
                        onBackPressed()
                    }
                }

                ActionViewModel.State.FAILED -> {
                    binding.closeBtn.apply {
                        if (!this.isVisible) this.show()
                        if (!this.isFocused) this.requestFocus()
                    }
                }

                else -> {}
            }
        }
    }

    override fun onCreateMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_flash, menu)
    }

    override fun onMenuItemSelected(item: MenuItem): Boolean {
        return viewModel.onMenuItemClicked(item)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        defaultOrientation = activity?.requestedOrientation ?: -1
        activity?.requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LOCKED
        if (savedInstanceState == null) {
            viewModel.startRunAction()
        }
    }

    @SuppressLint("WrongConstant")
    override fun onDestroyView() {
        if (defaultOrientation != -1) {
            activity?.requestedOrientation = defaultOrientation
        }
        super.onDestroyView()
    }

    override fun onKeyEvent(event: KeyEvent): Boolean {
        return when (event.keyCode) {
            KeyEvent.KEYCODE_VOLUME_UP, KeyEvent.KEYCODE_VOLUME_DOWN -> true

            else -> false
        }
    }

    override fun onBackPressed(): Boolean {
        if (viewModel.state.value == ActionViewModel.State.RUNNING) return true
        return super.onBackPressed()
    }

    override fun onPreBind(binding: FragmentActionMd2Binding) = Unit
}
