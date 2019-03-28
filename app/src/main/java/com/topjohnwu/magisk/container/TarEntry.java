package com.topjohnwu.magisk.container;

import org.kamranzafar.jtar.TarHeader;

import java.io.File;
import java.util.Arrays;

public class TarEntry extends org.kamranzafar.jtar.TarEntry {

    public TarEntry(File file, String entryName) {
        super(file, entryName);
    }

    /*
    * Workaround missing java.nio.file.attribute.PosixFilePermission
    * Simply just assign a default permission to the file
    * */

    @Override
    public void extractTarHeader(String entryName) {
        int permissions = file.isDirectory() ? 000755 : 000644;
        header = TarHeader.createHeader(entryName, file.length(), file.lastModified() / 1000, file.isDirectory(), permissions);
        header.userName = new StringBuffer("");
        header.groupName = header.userName;
    }

    /*
    * Rewrite the header to GNU format
    * */

    @Override
    public void writeEntryHeader(byte[] outbuf) {
        super.writeEntryHeader(outbuf);

        System.arraycopy("ustar  \0".getBytes(), 0, outbuf, 257, TarHeader.USTAR_MAGICLEN);
        getOctalBytes(header.mode, outbuf, 100, TarHeader.MODELEN);
        getOctalBytes(header.userId, outbuf, 108, TarHeader.UIDLEN);
        getOctalBytes(header.groupId, outbuf, 116, TarHeader.GIDLEN);
        getOctalBytes(header.size, outbuf, 124, TarHeader.SIZELEN);
        getOctalBytes(header.modTime, outbuf, 136, TarHeader.MODTIMELEN);
        Arrays.fill(outbuf, 148, 148 + TarHeader.CHKSUMLEN, (byte) ' ');
        Arrays.fill(outbuf, 329, 329 + TarHeader.USTAR_DEVLEN, (byte) '\0');
        Arrays.fill(outbuf, 337, 337 + TarHeader.USTAR_DEVLEN, (byte) '\0');

        // Recalculate checksum
        getOctalBytes(computeCheckSum(outbuf), outbuf, 148, TarHeader.CHKSUMLEN);
    }

    /*
    * Proper octal to ASCII conversion
    * */

    private void getOctalBytes(long value, byte[] buf, int offset, int length) {
        int idx = length - 1;

        buf[offset + idx] = 0;
        --idx;

        for (long val = value; idx >= 0; --idx) {
            buf[offset + idx] = (byte) ((byte) '0' + (byte) (val & 7));
            val = val >> 3;
        }
    }
}
