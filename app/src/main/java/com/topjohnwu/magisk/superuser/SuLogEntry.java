package com.topjohnwu.magisk.superuser;

import android.content.ContentValues;
import android.database.Cursor;
import android.os.Parcel;
import android.os.Parcelable;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class SuLogEntry implements Parcelable {

    public int fromUid, toUid, fromPid;
    public String packageName, appName, command;
    public boolean action;
    public Date date;

    public SuLogEntry(Policy policy) {
        fromUid = policy.uid;
        packageName = policy.packageName;
        appName = policy.appName;
    }

    public SuLogEntry(Cursor c) {
        fromUid = c.getInt(c.getColumnIndex("from_uid"));
        fromPid = c.getInt(c.getColumnIndex("from_pid"));
        toUid = c.getInt(c.getColumnIndex("to_uid"));
        packageName = c.getString(c.getColumnIndex("package_name"));
        appName = c.getString(c.getColumnIndex("app_name"));
        command = c.getString(c.getColumnIndex("command"));
        action = c.getInt(c.getColumnIndex("action")) != 0;
        date = new Date(c.getLong(c.getColumnIndex("time")) * 1000);
    }

    public ContentValues getContentValues() {
        ContentValues values = new ContentValues();
        values.put("from_uid", fromUid);
        values.put("package_name", packageName);
        values.put("app_name", appName);
        values.put("from_pid", fromPid);
        values.put("command", command);
        values.put("to_uid", toUid);
        values.put("action", action ? 1 : 0);
        values.put("time", date.getTime() / 1000);
        return values;
    }

    public String getDateString() {
        return DateFormat.getDateInstance(DateFormat.MEDIUM).format(date);
    }

    public String getTimeString() {
        return new SimpleDateFormat("h:mm a", Locale.US).format(date);
    }


    public static final Creator<SuLogEntry> CREATOR = new Creator<SuLogEntry>() {
        @Override
        public SuLogEntry createFromParcel(Parcel in) {
            return new SuLogEntry(in);
        }

        @Override
        public SuLogEntry[] newArray(int size) {
            return new SuLogEntry[size];
        }
    };

    protected SuLogEntry(Parcel in) {
        fromUid = in.readInt();
        toUid = in.readInt();
        fromPid = in.readInt();
        packageName = in.readString();
        appName = in.readString();
        command = in.readString();
        action = in.readByte() != 0;
        date = new Date(in.readLong());
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(fromUid);
        dest.writeInt(toUid);
        dest.writeInt(fromPid);
        dest.writeString(packageName);
        dest.writeString(appName);
        dest.writeString(command);
        dest.writeByte((byte) (action ? 1 : 0));
        dest.writeLong(date.getTime());
    }
}
