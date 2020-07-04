
import org.gradle.api.GradleException
import org.gradle.api.Plugin
import org.gradle.api.Project
import java.io.File
import java.util.*

object Deps {
    const val vKotlin = "1.3.72"
    const val vNav = "2.3.0"
}

private val props = Properties()

object Config {
    operator fun get(key: String) = props[key] as? String
    fun contains(key: String) = props.containsKey(key)
}

class MagiskPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        val file = project.findProperty("configPath")?.let { File(it as String) }
            ?: project.file("config.prop")
        if (!file.exists())
            throw GradleException("Please setup config.prop")

        file.inputStream().use { props.load(it) }
    }
}
