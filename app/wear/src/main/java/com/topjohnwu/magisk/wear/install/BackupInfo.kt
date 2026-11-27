package com.topjohnwu.magisk.wear.install

import com.topjohnwu.magisk.core.ktx.await
import com.topjohnwu.magisk.terminal.shellQuoted
import com.topjohnwu.superuser.Shell
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

/**
 * The stock boot image backup that Magisk saved when it patched the installed boot image,
 * which restoring the images flashes back.
 *
 * @param sha1 the SHA1 of the stock image, as recorded in Magisk's config, or null if none is
 * @param exists whether the backup is there to be restored
 */
data class BackupInfo(val sha1: String?, val exists: Boolean) {

    companion object {

        private const val SHA1_PREFIX = "SHA1="

        /** Directory of the backup of the image with [sha1], see restore_imgs in the scripts */
        fun backupDir(sha1: String) = "/data/magisk_backup_$sha1"

        /**
         * Read the backup from the content of `$MAGISKTMP/.magisk/config`. Like grep_prop in
         * the scripts, the first `SHA1=` line counts. [backupDirExists] is asked about the
         * backup directory only when there is a SHA1.
         */
        fun parse(config: String, backupDirExists: (String) -> Boolean): BackupInfo {
            val sha1 = config.lineSequence()
                .firstOrNull { it.startsWith(SHA1_PREFIX) }
                ?.removePrefix(SHA1_PREFIX)
                // The scripts run the config through dos2unix
                ?.trimEnd('\r')
                ?.ifEmpty { null }
                ?: return BackupInfo(null, false)
            return BackupInfo(sha1, backupDirExists(backupDir(sha1)))
        }

        /** Read the backup through the root shell. Without root, there is none. */
        suspend fun read(): BackupInfo = withContext(Dispatchers.IO) {
            val config = Shell.cmd("cat \"\$(magisk --path)/.magisk/config\"").await()
            if (!config.isSuccess) return@withContext BackupInfo(null, false)
            parse(config.out.joinToString("\n")) { dir ->
                // Same checks as restore_imgs, which fails without the image in the directory
                val quoted = dir.shellQuoted()
                Shell.cmd("[ -d $quoted ] && [ -f $quoted/boot.img.gz ]").exec().isSuccess
            }
        }
    }
}
