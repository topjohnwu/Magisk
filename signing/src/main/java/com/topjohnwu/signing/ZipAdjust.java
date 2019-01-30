package com.topjohnwu.signing;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class ZipAdjust {

    public static void adjust(File input, File output) throws IOException {
        try (
            RandomAccessFile in = new RandomAccessFile(input, "r");
            FileOutputStream out = new FileOutputStream(output)
        ) {
            adjust(in, out);
        }
    }

    public static void adjust(RandomAccessFile in, OutputStream out) throws IOException {

        CentralFooter footer = new CentralFooter(in);
        int outOff = 0;
        long centralOff = unsigned(footer.centralDirectoryOffset);
        CentralHeader[] centralHeaders = new CentralHeader[unsigned(footer.numEntries)];

        // Loop through central directory entries
        for (int i = 0; i < centralHeaders.length; ++i) {
            // Read central header
            in.seek(centralOff);
            centralHeaders[i] = new CentralHeader(in);
            centralOff = in.getFilePointer();

            // Read local header
            in.seek(unsigned(centralHeaders[i].localHeaderOffset));
            LocalHeader localHeader = new LocalHeader(in);

            // Make sure local and central headers matches, and strip out data descriptor flag
            centralHeaders[i].localHeaderOffset = outOff;
            centralHeaders[i].flags &= ~(1 << 3);
            localHeader.flags = centralHeaders[i].flags;
            localHeader.crc32 = centralHeaders[i].crc32;
            localHeader.compressedSize = centralHeaders[i].compressedSize;
            localHeader.uncompressedSize = centralHeaders[i].uncompressedSize;
            localHeader.fileNameLength = centralHeaders[i].fileNameLength;
            localHeader.filename = centralHeaders[i].filename;

            // Write local header
            outOff += localHeader.write(out);

            // Copy data
            int read;
            long len = unsigned(localHeader.compressedSize);
            outOff += len;
            byte data[] = new byte[4096];
            while ((read = in.read(data, 0,
                    len < data.length ? (int) len : data.length)) > 0) {
                out.write(data, 0, read);
                len -= read;
            }
        }

        footer.centralDirectoryOffset = outOff;

        // Write central directory
        outOff = 0;
        for (CentralHeader header : centralHeaders)
            outOff += header.write(out);

        // Write central directory record
        footer.centralDirectorySize = outOff;
        footer.write(out);
    }

    public static short unsigned(byte n) {
        return (short)(n & 0xff);
    }

    public static int unsigned(short n) {
        return n & 0xffff;
    }

    public static long unsigned(int n) {
        return n & 0xffffffffL;
    }

    public static class CentralFooter {

        static final int MAGIC =  0x06054b50;
        
        int signature;
        short diskNumber;
        short centralDirectoryDiskNumber;
        short numEntriesThisDisk;
        short numEntries;
        int centralDirectorySize;
        int centralDirectoryOffset;
        short zipCommentLength;
        // byte[] comments;

        CentralFooter(RandomAccessFile file) throws IOException {
            byte[] buffer = new byte[22];
            for (long i = file.length() - 4; i >= 0; --i) {
                file.seek(i);
                file.read(buffer, 0 ,4);
                ByteBuffer buf = ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN);
                signature = buf.getInt();
                if (signature != MAGIC) {
                    continue;
                }
                file.read(buffer, 4, buffer.length - 4);
                diskNumber = buf.getShort();
                centralDirectoryDiskNumber = buf.getShort();
                numEntriesThisDisk = buf.getShort();
                numEntries = buf.getShort();
                centralDirectorySize = buf.getInt();
                centralDirectoryOffset = buf.getInt();
                zipCommentLength = buf.getShort();
                break;
            }
        }

        int write(OutputStream out) throws IOException {
            byte[] buffer = new byte[22];
            ByteBuffer buf = ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN);
            buf.putInt(signature);
            buf.putShort(diskNumber);
            buf.putShort(centralDirectoryDiskNumber);
            buf.putShort(numEntriesThisDisk);
            buf.putShort(numEntries);
            buf.putInt(centralDirectorySize);
            buf.putInt(centralDirectoryOffset);
            buf.putShort((short) 0);  // zipCommentLength
            out.write(buffer);
            return buffer.length;
        }
    }

    static class CentralHeader {

        static final int MAGIC =  0x02014b50;

        int signature;
        short versionMadeBy;
        short versionNeededToExtract;
        short flags;
        short compressionMethod;
        short lastModFileTime;
        short lastModFileDate;
        int crc32;
        int compressedSize;
        int uncompressedSize;
        short fileNameLength;
        short extraFieldLength;
        short fileCommentLength;
        short diskNumberStart;
        short internalFileAttributes;
        int externalFileAttributes;
        int localHeaderOffset;
        byte[] filename;
        // byte[] extra;
        // byte[] comment;

        CentralHeader(RandomAccessFile file) throws IOException {
            byte[] buffer = new byte[46];
            file.read(buffer);
            ByteBuffer buf = ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN);
            signature = buf.getInt();
            if (signature != MAGIC)
                throw new IOException();
            versionMadeBy = buf.getShort();
            versionNeededToExtract = buf.getShort();
            flags = buf.getShort();
            compressionMethod = buf.getShort();
            lastModFileTime = buf.getShort();
            lastModFileDate = buf.getShort();
            crc32 = buf.getInt();
            compressedSize = buf.getInt();
            uncompressedSize = buf.getInt();
            fileNameLength = buf.getShort();
            extraFieldLength = buf.getShort();
            fileCommentLength = buf.getShort();
            diskNumberStart = buf.getShort();
            internalFileAttributes = buf.getShort();
            externalFileAttributes = buf.getInt();
            localHeaderOffset = buf.getInt();
            filename = new byte[unsigned(fileNameLength)];
            file.read(filename);
            file.skipBytes(unsigned(extraFieldLength) + unsigned(fileCommentLength));
        }

        int write(OutputStream out) throws IOException {
            byte[] buffer = new byte[46];
            ByteBuffer buf = ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN);
            buf.putInt(signature);
            buf.putShort(versionMadeBy);
            buf.putShort(versionNeededToExtract);
            buf.putShort(flags);
            buf.putShort(compressionMethod);
            buf.putShort(lastModFileTime);
            buf.putShort(lastModFileDate);
            buf.putInt(crc32);
            buf.putInt(compressedSize);
            buf.putInt(uncompressedSize);
            buf.putShort(fileNameLength);
            buf.putShort((short) 0); // extraFieldLength
            buf.putShort((short) 0); // fileCommentLength
            buf.putShort(diskNumberStart);
            buf.putShort(internalFileAttributes);
            buf.putInt(externalFileAttributes);
            buf.putInt(localHeaderOffset);
            out.write(buffer);
            out.write(filename);
            return buffer.length + filename.length;
        }
    }

    static class LocalHeader {

        static final int MAGIC = 0x04034b50;

        int signature;
        short versionNeededToExtract;
        short flags;
        short compressionMethod;
        short lastModFileTime;
        short lastModFileDate;
        int crc32;
        int compressedSize;
        int uncompressedSize;
        short fileNameLength;
        short extraFieldLength;
        byte[] filename;
        // byte[] extra;

        LocalHeader(RandomAccessFile file) throws IOException {
            byte[] buffer = new byte[30];
            file.read(buffer);
            ByteBuffer buf = ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN);
            signature = buf.getInt();
            if (signature != MAGIC)
                throw new IOException();
            versionNeededToExtract = buf.getShort();
            flags = buf.getShort();
            compressionMethod = buf.getShort();
            lastModFileTime = buf.getShort();
            lastModFileDate = buf.getShort();
            crc32 = buf.getInt();
            compressedSize = buf.getInt();
            uncompressedSize = buf.getInt();
            fileNameLength = buf.getShort();
            extraFieldLength = buf.getShort();
            file.skipBytes(unsigned(fileNameLength) + unsigned(extraFieldLength));
        }

        int write(OutputStream out) throws IOException {
            byte[] buffer = new byte[30];
            ByteBuffer buf = ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN);
            buf.putInt(signature);
            buf.putShort(versionNeededToExtract);
            buf.putShort(flags);
            buf.putShort(compressionMethod);
            buf.putShort(lastModFileTime);
            buf.putShort(lastModFileDate);
            buf.putInt(crc32);
            buf.putInt(compressedSize);
            buf.putInt(uncompressedSize);
            buf.putShort(fileNameLength);
            buf.putShort((short) 0); // extraFieldLength
            out.write(buffer);
            out.write(filename);
            return buffer.length + filename.length;
        }
    }
}
