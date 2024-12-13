package com.topjohnwu.magisk.test

import androidx.test.ext.junit.runners.AndroidJUnit4
import com.topjohnwu.magisk.core.TestImpl
import org.junit.BeforeClass
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class MagiskAppTest {

    companion object {
        @BeforeClass
        @JvmStatic
        fun before() {
            TestImpl.before()
        }
    }

    @Test
    fun testZygisk() {
        TestImpl.testZygisk()
    }
}
