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

/*
* A universal random access interface for both JarFile and JarInputStream
*
* In the case when JarInputStream is provided to constructor, the whole stream
* will be loaded into memory for random access purposes.
* On the other hand, when a JarFile is provided, it simply works as a wrapper.
* */

public class JarMap implements Closeable, AutoCloseable {

    private JarFile jarFile;
    private JarInputStream jis;
    private LinkedHashMap<String, JarEntry> bufMap;
    private Manifest manifest;

    public JarMap(File file) throws IOException {
        this(file, true);
    }

    public JarMap(File file, boolean verify) throws IOException {
        this(file, verify, ZipFile.OPEN_READ);
    }

    public JarMap(File file, boolean verify, int mode) throws IOException {
        jarFile = new JarFile(file, verify, mode);
        manifest = jarFile.getManifest();
    }

    public JarMap(String name) throws IOException {
        this(new File(name));
    }

    public JarMap(String name, boolean verify) throws IOException {
        this(new File(name), verify);
    }

    public JarMap(InputStream is) throws IOException {
        this(is, true);
    }

    public JarMap(InputStream is, boolean verify) throws IOException {
        jis = new JarInputStream(is, verify);
        bufMap = new LinkedHashMap<>();
        JarEntry entry;
        while ((entry = jis.getNextJarEntry()) != null) {
            bufMap.put(entry.getName(), new JarMapEntry(entry, jis));
        }
        manifest = jis.getManifest();
    }

    public File getFile() {
        return jarFile == null ? null : new File(jarFile.getName());
    }

    public Manifest getManifest() {
        return manifest;
    }

    public InputStream getInputStream(ZipEntry ze) throws IOException {
        if (bufMap != null) {
            JarMapEntry e = (JarMapEntry) bufMap.get(ze.getName());
            if (e != null)
                return e.data.getInputStream();
        }
        return jarFile.getInputStream(ze);
    }

    public OutputStream getOutputStream(ZipEntry ze) {
        manifest = null; /* Invalidate the manifest */
        if (bufMap == null)
            bufMap = new LinkedHashMap<>();
        JarMapEntry e = new JarMapEntry(ze.getName());
        bufMap.put(ze.getName(), e);
        return e.data;
    }

    public byte[] getRawData(ZipEntry ze) throws IOException {
        if (bufMap != null) {
            JarMapEntry e = (JarMapEntry) bufMap.get(ze.getName());
            if (e != null)
                return e.data.toByteArray();
        }
        ByteArrayStream bytes = new ByteArrayStream();
        bytes.readFrom(jarFile.getInputStream(ze));
        return bytes.toByteArray();
    }

    public Enumeration<JarEntry> entries() {
        return jarFile == null ? Collections.enumeration(bufMap.values()) : jarFile.entries();
    }

    public ZipEntry getEntry(String name) {
        return getJarEntry(name);
    }

    public JarEntry getJarEntry(String name) {
        JarEntry e = jarFile == null ? bufMap.get(name) : jarFile.getJarEntry(name);
        if (e == null && bufMap != null)
            return bufMap.get(name);
        return e;
    }

    @Override
    public void close() throws IOException {
        (jarFile == null ? jis : jarFile).close();
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
