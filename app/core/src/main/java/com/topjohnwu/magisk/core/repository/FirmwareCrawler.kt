package com.topjohnwu.magisk.core.repository

import android.os.Build
import com.topjohnwu.magisk.core.AppContext
import com.topjohnwu.magisk.core.Info
import com.topjohnwu.magisk.core.model.GooglebookRecoveryJson
import timber.log.Timber
import com.topjohnwu.magisk.core.R as CoreR

class FirmwareCrawler(
    private val svc: NetworkService,
) {

    private val providers: Map<() -> Boolean, Provider> = mapOf(
        { Info.isGooglebook } to GooglebookProvider,
    )

    data class Candidate(
        val url: String,
        val title: String,
        val description: String = "",
    )

    fun interface Provider {
        suspend fun crawl(svc: NetworkService): Candidate?
    }

    suspend fun crawl(): Candidate? {
        for ((predicate, provider) in providers) {
            try {
                if (predicate()) {
                    val candidate = provider.crawl(svc)
                    if (candidate != null) return candidate
                }
            } catch (e: Exception) {
                Timber.e(e)
            }
        }
        return null
    }

    private object GooglebookProvider : Provider {
        override suspend fun crawl(svc: NetworkService): Candidate? {
            val list = svc.fetchGooglebookRecovery() ?: return null
            val matching = list.filter {
                it.device.equals(Build.DEVICE, ignoreCase = true) ||
                        it.model.equals(Build.MODEL, ignoreCase = true)
            }
            val recovery = matching.maxWithOrNull(
                compareBy<GooglebookRecoveryJson> { it.dateupdated }
                    .thenBy { it.previousVersionGeneration }
            ) ?: return null

            if (recovery.url.isEmpty()) return null

            val details = buildString {
                if (recovery.model.isNotEmpty()) {
                    append(recovery.model)
                } else if (recovery.name.isNotEmpty()) {
                    append(recovery.name)
                }
                if (recovery.buildidentifier.isNotEmpty()) {
                    if (isNotEmpty()) append(" • ")
                    append(recovery.buildidentifier)
                }
            }
            return Candidate(
                url = recovery.url,
                title = AppContext.getString(CoreR.string.download_googlebook_recovery_image),
                description = details,
            )
        }
    }
}
