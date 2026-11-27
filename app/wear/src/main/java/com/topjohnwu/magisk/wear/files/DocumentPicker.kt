package com.topjohnwu.magisk.wear.files

import android.content.Context
import android.content.Intent
import android.net.Uri
import androidx.activity.result.contract.ActivityResultContract
import androidx.activity.result.contract.ActivityResultContracts

// Packages that register ACTION_OPEN_DOCUMENT on devices without a document picker, only to
// tell the user that no app can handle it. Wear OS ships the first one.
private val StubPackages = setOf(
    "com.google.android.wearable.frameworkpackagestubs",
    "com.android.frameworkpackagestubs",
)

/** The package of an activity that handles ACTION_OPEN_DOCUMENT, and whether it is a system app. */
class PickerHandler(val packageName: String, val isSystem: Boolean)

/**
 * The package to open documents with, chosen from the [handlers] of ACTION_OPEN_DOCUMENT in
 * resolution order, or null if none of them is a real picker.
 *
 * Only a system app is used. The picker is sent to one package without a chooser, and on a
 * watch without a real picker any installed app could otherwise register for the action and
 * hand over the module zip or boot image to install.
 */
fun documentPickerPackage(handlers: List<PickerHandler>): String? =
    handlers.firstOrNull { it.isSystem && it.packageName !in StubPackages }?.packageName

/** A request to pick a document of one of [mimeTypes] with the picker in [packageName]. */
class DocumentRequest(val packageName: String, val mimeTypes: Array<String>)

/**
 * ACTION_OPEN_DOCUMENT sent to the package of the request, so a stub that also handles it can
 * never be chosen instead.
 */
class OpenDocumentIn : ActivityResultContract<DocumentRequest, Uri?>() {
    private val openDocument = ActivityResultContracts.OpenDocument()

    override fun createIntent(context: Context, input: DocumentRequest): Intent =
        openDocument.createIntent(context, input.mimeTypes).setPackage(input.packageName)

    override fun parseResult(resultCode: Int, intent: Intent?): Uri? =
        openDocument.parseResult(resultCode, intent)
}
