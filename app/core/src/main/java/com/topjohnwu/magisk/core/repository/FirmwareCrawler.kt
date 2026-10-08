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
        { Info.isPixel } to PixelProvider,
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

    internal object PixelProvider : Provider {
        override suspend fun crawl(svc: NetworkService): Candidate? =
            crawl(svc, Build.DEVICE, Build.MODEL, Build.ID)

        suspend fun crawl(
            svc: NetworkService,
            device: String,
            model: String = Build.MODEL,
            buildId: String = "",
        ): Candidate? {
            val html = svc.fetchPixelFactoryImages() ?: return null
            val sectionRegex = Regex(
                """<h2 id="${Regex.escape(device)}"[^>]*>(.*?)</h2>\s*<table>(.*?)</table>""",
                setOf(RegexOption.DOT_MATCHES_ALL, RegexOption.IGNORE_CASE)
            )
            val match = sectionRegex.find(html) ?: return null
            val heading = match.groupValues[1]
            val table = match.groupValues[2]

            val parsedModel = Regex("""for\s+([^<]+)$""").find(heading)
                ?.groupValues?.get(1)?.trim()?.trim('"', '\'') ?: model
            if (!parsedModel.startsWith("Pixel", ignoreCase = true)) return null

            val rowRegex = Regex(
                """<tr[^>]*>\s*<td>([^<]+)</td>.*?href="(https://dl\.google\.com/dl/android/aosp/[^"]+)"""",
                RegexOption.DOT_MATCHES_ALL
            )
            val rows = rowRegex.findAll(table).toList()
            if (rows.isEmpty()) return null

            val target = if (buildId.isNotEmpty()) {
                rows.findLast { it.value.contains(buildId, ignoreCase = true) } ?: rows.last()
            } else {
                rows.last()
            }

            val version = target.groupValues[1].trim()
                .replace("&amp;", "&")
                .replace("&quot;", "\"")

            return Candidate(
                url = target.groupValues[2],
                title = AppContext.getString(CoreR.string.download_pixel_factory_image),
                description = "$parsedModel • $version",
            )
        }
    }
}
