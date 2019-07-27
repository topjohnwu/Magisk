package com.topjohnwu.magisk.model.entity

import androidx.annotation.WorkerThread
import com.topjohnwu.magisk.Const
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.io.SuFile

abstract class BaseModule : Comparable<BaseModule> {
    abstract var id: String
        protected set
    abstract var name: String
        protected set
    abstract var author: String
        protected set
    abstract var version: String
        protected set
    abstract var versionCode: Int
        protected set
    abstract var description: String
        protected set

    @Throws(NumberFormatException::class)
    protected fun parseProps(props: List<String>) {
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
            }
        }
    }

    override operator fun compareTo(other: BaseModule) = name.compareTo(other.name, true)
}

class Module(path: String) : BaseModule() {
    override var id: String = ""
    override var name: String = ""
    override var author: String = ""
    override var version: String = ""
    override var versionCode: Int = -1
    override var description: String = ""

    private val removeFile: SuFile = SuFile(path, "remove")
    private val disableFile: SuFile = SuFile(path, "disable")
    private val updateFile: SuFile = SuFile(path, "update")

    val updated: Boolean = updateFile.exists()

    var enable: Boolean = !disableFile.exists()
        set(enable) {
            field = if (enable) {
                disableFile.delete()
            } else {
                !disableFile.createNewFile()
            }
        }

    var remove: Boolean = removeFile.exists()
        set(remove) {
            field = if (remove) {
                removeFile.createNewFile()
            } else {
                !removeFile.delete()
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
            name = id;
        }
    }

    companion object {

        @WorkerThread
        fun loadModules(): List<Module> {
            val moduleList = mutableListOf<Module>()
            val path = SuFile(Const.MAGISK_PATH)
            val modules =
                    path.listFiles { _, name -> name != "lost+found" && name != ".core" }.orEmpty()
            for (file in modules) {
                if (file.isFile) continue
                val module = Module(Const.MAGISK_PATH + "/" + file.name)
                moduleList.add(module)
            }
            return moduleList
        }
    }
}
