package com.topjohnwu.magisk.utils

import dalvik.system.DexClassLoader
import java.io.File
import java.io.IOException
import java.net.URL
import java.util.*

@Suppress("FunctionName")
inline fun <reified T> T.DynamicClassLoader(apk: File) = DynamicClassLoader(apk, T::class.java.classLoader)

class DynamicClassLoader(apk: File, parent: ClassLoader?)
    : DexClassLoader(apk.path, apk.parent, null, parent) {

    private val base by lazy { Any::class.java.classLoader!! }

    @Throws(ClassNotFoundException::class)
    override fun loadClass(name: String, resolve: Boolean) : Class<*>
        = findLoadedClass(name) ?: runCatching {
            base.loadClass(name)
        }.getOrElse {
            runCatching {
                findClass(name)
            }.getOrElse { err ->
                runCatching {
                    parent.loadClass(name)
                }.getOrElse { throw err }
            }
        }

    override fun getResource(name: String) = base.getResource(name)
            ?: findResource(name)
            ?: parent?.getResource(name)

    @Throws(IOException::class)
    override fun getResources(name: String): Enumeration<URL> {
        val resources = mutableListOf(
                base.getResources(name),
                findResources(name), parent.getResources(name))
        return object : Enumeration<URL> {
            override fun hasMoreElements(): Boolean {
                while (true) {
                    if (resources.isEmpty())
                        return false
                    if (!resources[0].hasMoreElements()) {
                        resources.removeAt(0)
                    } else {
                        return true
                    }
                }
            }

            override fun nextElement(): URL {
                if (!hasMoreElements())
                    throw NoSuchElementException()
                return resources[0].nextElement()
            }
        }
    }

}
