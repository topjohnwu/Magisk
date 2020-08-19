package com.topjohnwu.magisk.core.model.module

import android.os.Parcelable
import androidx.room.Entity
import androidx.room.PrimaryKey
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.data.repository.StringRepository
import com.topjohnwu.magisk.ktx.get
import com.topjohnwu.magisk.ktx.legalFilename
import kotlinx.android.parcel.Parcelize
import java.text.DateFormat
import java.util.*

@Entity(tableName = "repos")
@Parcelize
data class Repo(
    @PrimaryKey override var id: String,
    override var name: String,
    override var author: String,
    override var version: String,
    override var versionCode: Int,
    override var description: String,
    var last_update: Long
) : BaseModule(), Parcelable {

    private val stringRepo: StringRepository get() = get()

    val lastUpdate get() = Date(last_update)

    val lastUpdateString: String get() = dateFormat.format(lastUpdate)

    val downloadFilename: String get() = "$name-$version($versionCode).zip".legalFilename()

    suspend fun readme() = stringRepo.getReadme(this)

    val zipUrl: String get() = Const.Url.ZIP_URL.format(id)

    constructor(id: String) : this(id, "", "", "", -1, "", 0)

    @Throws(IllegalRepoException::class)
    private fun loadProps(props: String) {
        props.split("\\n".toRegex()).dropLastWhile { it.isEmpty() }.runCatching {
            parseProps(this)
        }.onFailure {
            throw IllegalRepoException("Repo [$id] parse error: " + it.message)
        }

        if (versionCode < 0) {
            throw IllegalRepoException("Repo [$id] does not contain versionCode")
        }
    }

    @Throws(IllegalRepoException::class)
    suspend fun update(lastUpdate: Date? = null) {
        lastUpdate?.let { last_update = it.time }
        loadProps(stringRepo.getMetadata(this))
    }

    class IllegalRepoException(message: String) : Exception(message)

    companion object {
        val dateFormat = DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.MEDIUM)
    }
}
