package com.topjohnwu.magisk.model.events

import android.Manifest
import android.app.Activity
import android.content.Intent
import androidx.annotation.RequiresPermission
import androidx.navigation.NavDirections
import com.topjohnwu.magisk.MainDirections
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.base.BaseActivity

class InstallExternalModuleEvent : ViewEvent(), ActivityExecutor {

    @RequiresPermission(allOf = [Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE])
    override fun invoke(activity: BaseActivity) {
        val intent = Intent(Intent.ACTION_GET_CONTENT)
        intent.type = "application/zip"
        activity.startActivityForResult(intent, Const.ID.FETCH_ZIP)
    }

    companion object {

        fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?): NavDirections? {
            if (requestCode == Const.ID.FETCH_ZIP && resultCode == Activity.RESULT_OK && data != null) {
                val data = data.data
                if (data != null) {
                    return MainDirections.actionFlashFragment(data, Const.Value.FLASH_ZIP)
                }
            }
            return null
        }

    }

}
