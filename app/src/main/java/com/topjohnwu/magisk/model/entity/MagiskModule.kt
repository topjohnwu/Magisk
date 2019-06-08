package com.topjohnwu.magisk.model.entity

import android.os.Parcelable
import androidx.annotation.AnyThread
import androidx.annotation.NonNull
import androidx.annotation.WorkerThread
import androidx.room.Entity
import androidx.room.PrimaryKey
import com.topjohnwu.magisk.Const
import com.topjohnwu.magisk.data.database.base.su
import io.reactivex.Single
import kotlinx.android.parcel.Parcelize
import okhttp3.ResponseBody
import java.io.File

interface MagiskModule : Parcelable {
    val id: String
    val name: String
    val author: String
    val version: String
    val versionCode: String
    val description: String
}

@Entity(tableName = "repos")
@Parcelize
data class Repository(
    @PrimaryKey @NonNull
    override val id: String,
    override val name: String,
    override val author: String,
    override val version: String,
    override val versionCode: String,
    override val description: String,
    val lastUpdate: Long
) : MagiskModule

@Parcelize
data class Module(
    override val id: String,
    override val name: String,
    override val author: String,
    override val version: String,
    override val versionCode: String,
    override val description: String,
    val path: String
) : MagiskModule

@AnyThread
fun File.toModule(): Single<Module> {
    val path = "${Const.MAGISK_PATH}/$name"
    return "dos2unix < $path/module.prop".su()
        .map { it.first().toModule(path) }
}

fun Map<String, String>.toModule(path: String): Module {
    return Module(
        id = get("id").orEmpty(),
        name = get("name").orEmpty(),
        author = get("author").orEmpty(),
        version = get("version").orEmpty(),
        versionCode = get("versionCode").orEmpty(),
        description = get("description").orEmpty(),
        path = path
    )
}

@WorkerThread
fun ResponseBody.toRepository(lastUpdate: Long) = string()
    .split(Regex("\\n"))
    .map { it.split("=", limit = 2) }
    .filter { it.size == 2 }
    .map { Pair(it[0], it[1]) }
    .toMap()
    .toRepository(lastUpdate)

@AnyThread
fun Map<String, String>.toRepository(lastUpdate: Long) = Repository(
    id = get("id").orEmpty(),
    name = get("name").orEmpty(),
    author = get("author").orEmpty(),
    version = get("version").orEmpty(),
    versionCode = get("versionCode").orEmpty(),
    description = get("description").orEmpty(),
    lastUpdate = lastUpdate
)