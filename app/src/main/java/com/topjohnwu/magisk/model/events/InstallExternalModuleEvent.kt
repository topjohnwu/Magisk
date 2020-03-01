package com.topjohnwu.magisk.model.events

import android.Manifest
import android.app.Activity
import android.content.Context
import android.content.Intent
import androidx.annotation.RequiresPermission
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.core.base.BaseActivity
import com.topjohnwu.magisk.core.intent
import com.topjohnwu.magisk.legacy.flash.FlashActivity

class InstallExternalModuleEvent : ViewEvent(), ActivityExecutor {

    @RequiresPermission(allOf = [Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE])
    override fun invoke(activity: BaseActivity) {
        val intent = Intent(Intent.ACTION_GET_CONTENT)
        intent.type = "application/zip"
        activity.startActivityForResult(intent, Const.ID.FETCH_ZIP)
    }

    companion object {

        fun onActivityResult(context: Context, requestCode: Int, resultCode: Int, data: Intent?) {
            if (requestCode == Const.ID.FETCH_ZIP && resultCode == Activity.RESULT_OK && data != null) {
                // Get the URI of the selected file
                val intent = context.intent<FlashActivity>()
                intent.setData(data.data).putExtra(Const.Key.FLASH_ACTION, Const.Value.FLASH_ZIP)
                context.startActivity(intent)
            }
        }

    }

}
