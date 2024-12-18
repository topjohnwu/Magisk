package com.topjohnwu.magisk.test

import android.os.Bundle
import androidx.test.platform.app.InstrumentationRegistry
import androidx.test.runner.AndroidJUnitRunner

class TestRunner : AndroidJUnitRunner() {
    override fun onCreate(arguments: Bundle) {
        // Force using the target context's classloader to run tests
        arguments.putString("classLoader", TestClassLoader::class.java.name)
        super.onCreate(arguments)
    }
}

private val targetClassLoader inline get() =
    InstrumentationRegistry.getInstrumentation().targetContext.classLoader

class TestClassLoader : ClassLoader(targetClassLoader)
