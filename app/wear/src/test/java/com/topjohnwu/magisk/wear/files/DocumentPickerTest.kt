package com.topjohnwu.magisk.wear.files

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Test

class DocumentPickerTest {

    private val wearStub = PickerHandler("com.google.android.wearable.frameworkpackagestubs", true)
    private val aospStub = PickerHandler("com.android.frameworkpackagestubs", true)
    private val documentsUi = PickerHandler("com.google.android.documentsui", true)
    private val userApp = PickerHandler("com.example.files", false)

    @Test
    fun noHandler() {
        assertNull(documentPickerPackage(emptyList()))
    }

    @Test
    fun onlyStub() {
        assertNull(documentPickerPackage(listOf(wearStub)))
        assertNull(documentPickerPackage(listOf(aospStub)))
    }

    @Test
    fun realPicker() {
        assertEquals(documentsUi.packageName, documentPickerPackage(listOf(documentsUi)))
    }

    @Test
    fun realPickerAfterStub() {
        assertEquals(documentsUi.packageName, documentPickerPackage(listOf(wearStub, documentsUi)))
    }

    @Test
    fun keepsResolutionOrder() {
        val otherSystem = PickerHandler("com.example.systemfiles", true)
        assertEquals(
            otherSystem.packageName,
            documentPickerPackage(listOf(otherSystem, documentsUi)),
        )
    }

    @Test
    fun skipsUserApps() {
        assertNull(documentPickerPackage(listOf(wearStub, userApp)))
        assertEquals(
            documentsUi.packageName,
            documentPickerPackage(listOf(userApp, documentsUi)),
        )
    }
}
