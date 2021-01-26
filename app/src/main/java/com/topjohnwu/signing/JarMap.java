package com.topjohnwu.signing;

import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Collections;
import java.util.Enumeration;
import java.util.LinkedHashMap;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.JarInputStream;
import java.util.jar.Manifest;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public abstract class JarMap implements Closeable {

    LinkedHashMap<String, JarEntry> entryMap;

    public static JarMap open(File file, boolean verify) throws IOException {
        return new FileMap(file, verify, ZipFile.OPEN_READ);
    }

    public static JarMap open(InputStream is, boolean verify) throws IOException {
        return new StreamMap(is, verify);
    }

    public File getFile() {
        return null;
    }

    public abstract Manifest getManifest() throws IOException;

    public InputStream getInputStream(ZipEntry ze) throws IOException {
        JarMapEntry e = getMapEntry(ze.getName());
        return e != null ? e.data.getInputStream() : null;
    }

    public OutputStream getOutputStream(ZipEntry ze) {
        if (entryMap == null)
            entryMap = new LinkedHashMap<>();
        JarMapEntry e = new JarMapEntry(ze.getName());
        entryMap.put(ze.getName(), e);
        return e.data;
    }

    public byte[] getRawData(ZipEntry ze) throws IOException {
        JarMapEntry e = getMapEntry(ze.getName());
        return e != null ? e.data.toByteArray() : null;
    }

    public abstract Enumeration<JarEntry> entries();

    public final ZipEntry getEntry(String name) {
        return getJarEntry(name);
    }

    public JarEntry getJarEntry(String name) {
        return getMapEntry(name);
    }

    JarMapEntry getMapEntry(String name) {
        JarMapEntry e = null;
        if (entryMap != null)
            e = (JarMapEntry) entryMap.get(name);
        return e;
    }

    private static class FileMap extends JarMap {

        private JarFile jarFile;

        FileMap(File file, boolean verify, int mode) throws IOException {
            jarFile = new JarFile(file, verify, mode);
        }

        @Override
        public File getFile() {
            return new File(jarFile.getName());
        }

        @Override
        public Manifest getManifest() throws IOException {
            return jarFile.getManifest();
        }

        @Override
        public InputStream getInputStream(ZipEntry ze) throws IOException {
            InputStream is = super.getInputStream(ze);
            return is != null ? is : jarFile.getInputStream(ze);
        }

        @Override
        public byte[] getRawData(ZipEntry ze) throws IOException {
            byte[] b = super.getRawData(ze);
            if (b != null)
                return b;
            ByteArrayStream bytes = new ByteArrayStream();
            bytes.readFrom(jarFile.getInputStream(ze));
            return bytes.toByteArray();
        }

        @Override
        public Enumeration<JarEntry> entries() {
            return jarFile.entries();
        }

        @Override
        public JarEntry getJarEntry(String name) {
            JarEntry e = getMapEntry(name);
            return e != null ? e : jarFile.getJarEntry(name);
        }

        @Override
        public void close() throws IOException {
            jarFile.close();
        }
    }

    private static class StreamMap extends JarMap {

        private JarInputStream jis;

        StreamMap(InputStream is, boolean verify) throws IOException {
            jis = new JarInputStream(is, verify);
            entryMap = new LinkedHashMap<>();
            JarEntry entry;
            while ((entry = jis.getNextJarEntry()) != null) {
                entryMap.put(entry.getName(), new JarMapEntry(entry, jis));
            }
        }

        @Override
        public Manifest getManifest() {
            return jis.getManifest();
        }

        @Override
        public Enumeration<JarEntry> entries() {
            return Collections.enumeration(entryMap.values());
        }

        @Override
        public void close() throws IOException {
            jis.close();
        }
    }

    private static class JarMapEntry extends JarEntry {

        ByteArrayStream data;

        JarMapEntry(JarEntry je, InputStream is) {
            super(je);
            data = new ByteArrayStream();
            data.readFrom(is);
        }

        JarMapEntry(String s) {
            super(s);
            data = new ByteArrayStream();
        }
    }
}
