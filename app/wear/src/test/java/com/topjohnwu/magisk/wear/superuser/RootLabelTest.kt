package com.topjohnwu.magisk.wear.superuser

import com.topjohnwu.magisk.core.model.su.SuPolicy
import com.topjohnwu.magisk.ui.superuser.PolicyItem
import org.junit.Assert.assertEquals
import org.junit.Test
import com.topjohnwu.magisk.core.R as CoreR

class RootLabelTest {

    // Built from a SuPolicy the same way SuperuserViewModel does
    private fun item(policy: SuPolicy) = PolicyItem(
        uid = policy.uid,
        packageName = "com.example.app",
        appName = "Example",
        isSharedUid = false,
        policyValue = policy.policy,
        notification = policy.notification,
        logging = policy.logging,
    )

    @Test
    fun allowIsFullRoot() {
        val label = rootLabel(item(SuPolicy(uid = 10123, policy = SuPolicy.ALLOW)))
        assertEquals(CoreR.string.wear_root_full, label)
    }

    @Test
    fun restrictIsRestrictedRoot() {
        val label = rootLabel(item(SuPolicy(uid = 10123, policy = SuPolicy.RESTRICT)))
        assertEquals(CoreR.string.wear_root_restricted, label)
    }

    @Test
    fun denyIsNoRoot() {
        val label = rootLabel(item(SuPolicy(uid = 10123, policy = SuPolicy.DENY)))
        assertEquals(CoreR.string.wear_root_none, label)
    }
}
