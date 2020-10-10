
import org.gradle.api.GradleException
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.kotlin.dsl.provideDelegate
import java.io.File
import java.util.*

private val props = Properties()

object Config {
    operator fun get(key: String) = props[key] as? String
    fun contains(key: String) = props.containsKey(key)
}

class MagiskPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        val configPath: String? by project
        val file = configPath?.let { File(it) } ?: project.file("config.prop")
        if (!file.exists())
            throw GradleException("Please setup config.prop")

        file.inputStream().use { props.load(it) }
    }
}
