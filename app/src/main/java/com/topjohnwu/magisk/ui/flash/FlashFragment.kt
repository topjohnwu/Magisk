package com.topjohnwu.magisk.ui.flash

import android.annotation.SuppressLint
import android.content.Context
import android.content.pm.ActivityInfo
import android.net.Uri
import android.os.Bundle
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import androidx.core.net.toUri
import androidx.navigation.NavDeepLinkBuilder
import com.topjohnwu.magisk.R
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.cmp
import com.topjohnwu.magisk.databinding.FragmentFlashMd2Binding
import com.topjohnwu.magisk.ui.MainActivity
import com.topjohnwu.magisk.ui.base.BaseUIActivity
import com.topjohnwu.magisk.ui.base.BaseUIFragment
import org.koin.androidx.viewmodel.ext.android.viewModel
import org.koin.core.parameter.parametersOf
import java.io.File
import com.topjohnwu.magisk.MainDirections.Companion.actionFlashFragment as toFlash
import com.topjohnwu.magisk.ui.flash.FlashFragmentArgs as args

class FlashFragment : BaseUIFragment<FlashViewModel, FragmentFlashMd2Binding>() {

    override val layoutRes = R.layout.fragment_flash_md2
    override val viewModel by viewModel<FlashViewModel> {
        parametersOf(args.fromBundle(requireArguments()))
    }

    private var defaultOrientation = -1

    override fun onStart() {
        super.onStart()
        setHasOptionsMenu(true)
        activity.setTitle(R.string.flash_screen_title)
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        inflater.inflate(R.menu.menu_flash, menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return viewModel.onMenuItemClicked(item)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        defaultOrientation = activity.requestedOrientation
        activity.requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_NOSENSOR
    }

    @SuppressLint("WrongConstant")
    override fun onDestroyView() {
        if (defaultOrientation != -1) {
            activity.requestedOrientation = defaultOrientation
        }
        super.onDestroyView()
    }

    override fun onBackPressed(): Boolean {
        if (viewModel.loading) return true
        return super.onBackPressed()
    }

    override fun onPreBind(binding: FragmentFlashMd2Binding) = Unit

    companion object {

        private fun createIntent(context: Context, args: args) =
            NavDeepLinkBuilder(context)
                .setGraph(R.navigation.main)
                .setComponentName(MainActivity::class.java.cmp(context.packageName))
                .setDestination(R.id.flashFragment)
                .setArguments(args.toBundle())
                .createPendingIntent()

        private fun flashType(isSecondSlot: Boolean) =
            if (isSecondSlot) Const.Value.FLASH_INACTIVE_SLOT else Const.Value.FLASH_MAGISK

        /* Flashing is understood as installing / flashing magisk itself */

        fun flashIntent(context: Context, file: File, isSecondSlot: Boolean, id: Int = -1) = args(
            installer = file.toUri(),
            action = flashType(isSecondSlot),
            dismissId = id
        ).let { createIntent(context, it) }

        fun flash(file: File, isSecondSlot: Boolean, id: Int) = toFlash(
            installer = file.toUri(),
            action = flashType(isSecondSlot),
            dismissId = id
        ).let { BaseUIActivity.postDirections(it) }

        /* Patching is understood as injecting img files with magisk */

        fun patchIntent(context: Context, file: File, uri: Uri, id: Int = -1) = args(
            installer = file.toUri(),
            action = Const.Value.PATCH_FILE,
            additionalData = uri,
            dismissId = id
        ).let { createIntent(context, it) }

        fun patch(file: File, uri: Uri, id: Int) = toFlash(
            installer = file.toUri(),
            action = Const.Value.PATCH_FILE,
            additionalData = uri,
            dismissId = id
        ).let { BaseUIActivity.postDirections(it) }

        /* Uninstalling is understood as removing magisk entirely */

        fun uninstallIntent(context: Context, file: File, id: Int = -1) = args(
            installer = file.toUri(),
            action = Const.Value.UNINSTALL,
            dismissId = id
        ).let { createIntent(context, it) }

        fun uninstall(file: File, id: Int) = toFlash(
            installer = file.toUri(),
            action = Const.Value.UNINSTALL,
            dismissId = id
        ).let { BaseUIActivity.postDirections(it) }

        /* Installing is understood as flashing modules / zips */

        fun installIntent(context: Context, file: File, id: Int = -1) = args(
            installer = file.toUri(),
            action = Const.Value.FLASH_ZIP,
            dismissId = id
        ).let { createIntent(context, it) }

        fun install(file: File, id: Int) = toFlash(
            installer = file.toUri(),
            action = Const.Value.FLASH_ZIP,
            dismissId = id
        ).let { BaseUIActivity.postDirections(it) }
    }

}
