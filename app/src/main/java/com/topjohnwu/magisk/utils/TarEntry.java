package com.topjohnwu.magisk.utils;

import org.kamranzafar.jtar.TarHeader;

import java.io.File;

public class TarEntry extends org.kamranzafar.jtar.TarEntry {

    public TarEntry(File file, String entryName) {
        super(file, entryName);
    }

    public TarEntry(byte[] headerBuf) {
        super(headerBuf);
    }

    /*
    * Workaround missing java.nio.file.attribute.PosixFilePermission
    * Simply just assign a default permission to the file
    * */

    @Override
    public void extractTarHeader(String entryName) {
        int permissions = file.isDirectory() ? 040755 : 0100644;
        header = TarHeader.createHeader(entryName, file.length(), file.lastModified() / 1000, file.isDirectory(), permissions);
    }
}
