package com.topjohnwu.magisk.core.model.module

import android.os.Parcelable
import androidx.room.Entity
import androidx.room.PrimaryKey
import com.topjohnwu.magisk.core.Const
import com.topjohnwu.magisk.data.repository.StringRepository
import com.topjohnwu.magisk.extensions.get
import com.topjohnwu.magisk.extensions.legalFilename
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

    val readme get() = stringRepo.getReadme(this)

    val zipUrl: String get() = Const.Url.ZIP_URL.format(id)

    constructor(id: String) : this(id, "", "", "", -1, "", 0)

    @Throws(IllegalRepoException::class)
    fun update() {
        val props = runCatching {
            stringRepo.getMetadata(this).blockingGet()
                    .orEmpty().split("\\n".toRegex()).dropLastWhile { it.isEmpty() }
        }.getOrElse {
            throw IllegalRepoException("Repo [$id] module.prop download error: " + it.message)
        }

        props.runCatching {
            parseProps(this)
        }.onFailure {
            throw IllegalRepoException("Repo [$id] parse error: " + it.message)
        }

        if (versionCode < 0) {
            throw IllegalRepoException("Repo [$id] does not contain versionCode")
        }
    }

    @Throws(IllegalRepoException::class)
    fun update(lastUpdate: Date) {
        last_update = lastUpdate.time
        update()
    }

    class IllegalRepoException(message: String) : Exception(message)

    companion object {
        val dateFormat = DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.MEDIUM)!!
    }
}
