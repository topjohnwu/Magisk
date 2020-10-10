package com.topjohnwu.magisk.core.model.module

import android.os.Parcelable
import androidx.room.Entity
import androidx.room.PrimaryKey
import com.topjohnwu.magisk.core.model.ModuleJson
import com.topjohnwu.magisk.data.repository.NetworkService
import com.topjohnwu.magisk.ktx.get
import com.topjohnwu.magisk.ktx.legalFilename
import kotlinx.android.parcel.Parcelize
import java.text.DateFormat
import java.util.*

@Entity(tableName = "repos")
@Parcelize
data class Repo(
    @PrimaryKey override var id: String,
    override var name: String = "",
    override var author: String = "",
    override var version: String = "",
    override var versionCode: Int = -1,
    override var description: String = "",
    val last_update: Long,
    val prop_url: String,
    val zip_url: String,
    val notes_url: String
) : BaseModule(), Parcelable {

    private val svc: NetworkService get() = get()

    constructor(info: ModuleJson) : this(
        id = info.id,
        last_update = info.last_update,
        prop_url = info.prop_url,
        zip_url = info.zip_url,
        notes_url = info.notes_url
    )

    val lastUpdate get() = Date(last_update)
    val lastUpdateString get() = DATE_FORMAT.format(lastUpdate)
    val downloadFilename get() = "$name-$version($versionCode).zip".legalFilename()

    suspend fun notes() = svc.fetchString(notes_url)

    @Throws(IllegalRepoException::class)
    suspend fun load() {
        val props = svc.fetchString(prop_url)
        props.split("\\n".toRegex()).dropLastWhile { it.isEmpty() }.runCatching {
            parseProps(this)
        }.onFailure {
            throw IllegalRepoException("Repo [$id] parse error: ", it)
        }

        if (versionCode < 0) {
            throw IllegalRepoException("Repo [$id] does not contain versionCode")
        }
    }

    class IllegalRepoException(msg: String, cause: Throwable? = null) : Exception(msg, cause)

    companion object {
        private val DATE_FORMAT = DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.MEDIUM)
    }

}
