package com.topjohnwu.magisk.data.database

import android.content.Context
import android.database.Cursor
import android.database.sqlite.SQLiteDatabase
import android.database.sqlite.SQLiteOpenHelper
import androidx.core.content.edit
import com.topjohnwu.magisk.Config
import com.topjohnwu.magisk.model.entity.Repo
import java.util.*

@Deprecated("")
class RepoDatabaseHelper
constructor(context: Context) : SQLiteOpenHelper(context, "repo.db", null, DATABASE_VER) {

    private val mDb: SQLiteDatabase = writableDatabase

    val rawCursor: Cursor
        @Deprecated("")
        get() = mDb.query(TABLE_NAME, null, null, null, null, null, null)

    val repoCursor: Cursor
        @Deprecated("")
        get() {
            var orderBy: String? = null
            when (Config.repoOrder) {
                Config.Value.ORDER_NAME -> orderBy = "name COLLATE NOCASE"
                Config.Value.ORDER_DATE -> orderBy = "last_update DESC"
            }
            return mDb.query(TABLE_NAME, null, null, null, null, null, orderBy)
        }

    val repoIDSet: Set<String>
        @Deprecated("")
        get() {
            val set = HashSet<String>(300)
            mDb.query(TABLE_NAME, null, null, null, null, null, null).use { c ->
                while (c.moveToNext()) {
                    set.add(c.getString(c.getColumnIndex("id")))
                }
            }
            return set
        }

    override fun onCreate(db: SQLiteDatabase) {
        onUpgrade(db, 0, DATABASE_VER)
    }

    override fun onUpgrade(db: SQLiteDatabase, oldVersion: Int, newVersion: Int) {
        if (oldVersion != newVersion) {
            // Nuke old DB and create new table
            db.execSQL("DROP TABLE IF EXISTS $TABLE_NAME")
            db.execSQL(
                    "CREATE TABLE IF NOT EXISTS " + TABLE_NAME + " " +
                            "(id TEXT, name TEXT, version TEXT, versionCode INT, " +
                            "author TEXT, description TEXT, last_update INT, PRIMARY KEY(id))")
            Config.prefs.edit {
                remove(Config.Key.ETAG_KEY)
            }
        }
    }

    override fun onDowngrade(db: SQLiteDatabase, oldVersion: Int, newVersion: Int) {
        onUpgrade(db, 0, DATABASE_VER)
    }

    @Deprecated("")
    fun clearRepo() {
        mDb.delete(TABLE_NAME, null, null)
    }


    @Deprecated("")
    fun removeRepo(id: String) {
        mDb.delete(TABLE_NAME, "id=?", arrayOf(id))
    }

    @Deprecated("")
    fun removeRepo(repo: Repo) {
        removeRepo(repo.id)
    }

    @Deprecated("")
    fun removeRepo(list: Iterable<String>) {
        list.forEach {
            mDb.delete(TABLE_NAME, "id=?", arrayOf(it))
        }
    }

    @Deprecated("")
    fun addRepo(repo: Repo) {
        mDb.replace(TABLE_NAME, null, repo.contentValues)
    }

    @Deprecated("")
    fun getRepo(id: String): Repo? {
        mDb.query(TABLE_NAME, null, "id=?", arrayOf(id), null, null, null).use { c ->
            if (c.moveToNext()) {
                return Repo(c)
            }
        }
        return null
    }

    companion object {
        private val DATABASE_VER = 5
        private val TABLE_NAME = "repos"
    }
}
