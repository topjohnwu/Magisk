package com.topjohnwu.magisk.wear.superuser

import androidx.annotation.StringRes
import com.topjohnwu.magisk.ui.superuser.PolicyItem
import com.topjohnwu.magisk.core.R as CoreR

/** The kind of root access [item] currently gets, shown under the app name. */
@StringRes
fun rootLabel(item: PolicyItem): Int = when {
    // Checked first: a restricted policy also counts as enabled
    item.isRestricted -> CoreR.string.wear_root_restricted
    item.isEnabled -> CoreR.string.wear_root_full
    else -> CoreR.string.wear_root_none
}
