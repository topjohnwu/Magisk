package com.topjohnwu.magisk.ui.component

import androidx.annotation.StringRes
import com.topjohnwu.magisk.core.R as CoreR

enum class AppSortBy(@StringRes val labelRes: Int) {
    NAME(CoreR.string.sort_by_name),
    PACKAGE_NAME(CoreR.string.sort_by_package_name),
    INSTALL_TIME(CoreR.string.sort_by_install_time),
    UPDATE_TIME(CoreR.string.sort_by_update_time);
}
