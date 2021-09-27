package com.topjohnwu.magisk.core.model.module

import android.os.Parcelable
import androidx.room.Entity
import androidx.room.PrimaryKey
import com.topjohnwu.magisk.core.model.ModuleJson
import com.topjohnwu.magisk.di.ServiceLocator
import com.topjohnwu.magisk.ktx.legalFilename
import kotlinx.parcelize.Parcelize
import java.text.DateFormat
import java.util.*

@Entity(tableName = "modules")
@Parcelize
data class OnlineModule(
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
) : Module(), Parcelable {

    private val svc get() = ServiceLocator.networkService

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
        try {
            val rawProps = svc.fetchString(prop_url)
            val props = rawProps.split("\\n".toRegex()).dropLastWhile { it.isEmpty() }
            parseProps(props)
        } catch (e: Exception) {
            throw IllegalRepoException("Repo [$id] parse error:", e)
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
