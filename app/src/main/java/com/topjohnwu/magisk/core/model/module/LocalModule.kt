package com.topjohnwu.magisk.core.model.module

import com.squareup.moshi.JsonDataException
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.io.SuFile
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.io.IOException
import java.util.*

data class LocalModule(
    private val path: String,
) : Module() {
    private val svc get() = ServiceLocator.networkService

    override var id: String = ""
    override var name: String = ""
    override var version: String = ""
    override var versionCode: Int = -1
    var author: String = ""
    var description: String = ""
    var updateInfo: OnlineModule? = null
    var outdated = false

    private var updateUrl: String = ""
    private val removeFile = SuFile(path, "remove")
    private val disableFile = SuFile(path, "disable")
    private val updateFile = SuFile(path, "update")
    private val ruleFile = SuFile(path, "sepolicy.rule")
    private val riruFolder = SuFile(path, "riru")
    private val zygiskFolder = SuFile(path, "zygisk")
    private val unloaded = SuFile(zygiskFolder, "unloaded")

    val updated: Boolean get() = updateFile.exists()
    val isRiru: Boolean get() = (id == "riru-core") || riruFolder.exists()
    val isZygisk: Boolean get() = zygiskFolder.exists()
    val zygiskUnloaded: Boolean get() = unloaded.exists()

    var enable: Boolean
        get() = !disableFile.exists()
        set(enable) {
            val dir = "$PERSIST/$id"
            if (enable) {
                disableFile.delete()
                if (Const.Version.atLeast_21_2())
                    Shell.su("copy_sepolicy_rules").submit()
                else
                    Shell.su("mkdir -p $dir", "cp -af $ruleFile $dir").submit()
            } else {
                !disableFile.createNewFile()
                if (Const.Version.atLeast_21_2())
                    Shell.su("copy_sepolicy_rules").submit()
                else
                    Shell.su("rm -rf $dir").submit()
            }
        }

    var remove: Boolean
        get() = removeFile.exists()
        set(remove) {
            if (remove) {
                if (updateFile.exists()) return
                removeFile.createNewFile()
                if (Const.Version.atLeast_21_2())
                    Shell.su("copy_sepolicy_rules").submit()
                else
                    Shell.su("rm -rf $PERSIST/$id").submit()
            } else {
                removeFile.delete()
                if (Const.Version.atLeast_21_2())
                    Shell.su("copy_sepolicy_rules").submit()
                else
                    Shell.su("cp -af $ruleFile $PERSIST/$id").submit()
            }
        }

    @Throws(NumberFormatException::class)
    private fun parseProps(props: List<String>) {
        for (line in props) {
            val prop = line.split("=".toRegex(), 2).map { it.trim() }
            if (prop.size != 2)
                continue

            val key = prop[0]
            val value = prop[1]
            if (key.isEmpty() || key[0] == '#')
                continue

            when (key) {
                "id" -> id = value
                "name" -> name = value
                "version" -> version = value
                "versionCode" -> versionCode = value.toInt()
                "author" -> author = value
                "description" -> description = value
                "updateJson" -> updateUrl = value
            }
        }
    }

    init {
        runCatching {
            parseProps(Shell.su("dos2unix < $path/module.prop").exec().out)
        }

        if (id.isEmpty()) {
            val sep = path.lastIndexOf('/')
            id = path.substring(sep + 1)
        }

        if (name.isEmpty()) {
            name = id
        }
    }

    suspend fun fetch(): Boolean {
        if (updateUrl.isEmpty())
            return false

        try {
            val json = svc.fetchModuleJson(updateUrl)
            updateInfo = OnlineModule(this, json)
            outdated = json.versionCode > versionCode
            return true
        } catch (e: IOException) {
            Timber.w(e)
        } catch (e: JsonDataException) {
            Timber.w(e)
        }

        return false
    }

    companion object {

        private val PERSIST get() = "${Const.MAGISKTMP}/mirror/persist/magisk"

        suspend fun installed() = withContext(Dispatchers.IO) {
            SuFile(Const.MAGISK_PATH)
                .listFiles()
                .orEmpty()
                .filter { !it.isFile }
                .map { LocalModule("${Const.MAGISK_PATH}/${it.name}") }
                .sortedBy { it.name.lowercase(Locale.ROOT) }
        }
    }
}
