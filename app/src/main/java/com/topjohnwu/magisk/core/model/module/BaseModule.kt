package com.topjohnwu.magisk.core.model.module

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
