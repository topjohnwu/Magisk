package com.topjohnwu.magisk.core.model.module

import androidx.annotation.WorkerThread
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.superuser.Shell
import com.topjohnwu.superuser.io.SuFile

class Module(path: String) : BaseModule() {
    override var id: String = ""
    override var name: String = ""
    override var author: String = ""
    override var version: String = ""
    override var versionCode: Int = -1
    override var description: String = ""

    private val removeFile = SuFile(path, "remove")
    private val disableFile = SuFile(path, "disable")
    private val updateFile = SuFile(path, "update")
    private val ruleFile = SuFile(path, "sepolicy.rule")

    val updated: Boolean get() = updateFile.exists()

    var enable: Boolean
        get() = !disableFile.exists()
        set(enable) {
            val dir = "$PERSIST/$id"
            if (enable) {
                Shell.su("mkdir -p $dir", "cp -af $ruleFile $dir").submit()
                disableFile.delete()
            } else {
                Shell.su("rm -rf $dir").submit()
                !disableFile.createNewFile()
            }
        }

    var remove: Boolean
        get() = removeFile.exists()
        set(remove) {
            if (remove) {
                Shell.su("rm -rf $PERSIST/$id").submit()
                removeFile.createNewFile()
            } else {
                Shell.su("cp -af $ruleFile $PERSIST/$id").submit()
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
            name = id
        }
    }

    companion object {

        private val PERSIST get() = "${Const.MAGISKTMP}/mirror/persist/magisk"

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
            return moduleList.sortedBy { it.name.toLowerCase() }
        }
    }
}
