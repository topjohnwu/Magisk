package com.topjohnwu.crypto;

import java.io.Closeable;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
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
    private InputStream is;
    private File file;
    private boolean isInputStream = false, hasLoaded = false, verify;
    private LinkedHashMap<String, JarEntry> bufMap = new LinkedHashMap<>();

    public JarMap(File file) throws IOException {
        this(file, true);
    }

    public JarMap(File file, boolean verify) throws IOException {
        this(file, verify, ZipFile.OPEN_READ);
    }

    public JarMap(File file, boolean verify, int mode) throws IOException {
        this.file = file;
        jarFile = new JarFile(file, verify, mode);
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
        isInputStream = true;
        this.is = is;
        this.verify = verify;
    }

    private void loadJarInputStream() {
        if (!isInputStream || hasLoaded) return;
        hasLoaded = true;
        JarEntry entry;
        try {
            jis = new JarInputStream(is, verify);
            while ((entry = jis.getNextJarEntry()) != null) {
                bufMap.put(entry.getName(), new JarMapEntry(entry, jis));
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public InputStream getInputStream() {
        try {
            return isInputStream ? is : new FileInputStream(file);
        } catch (FileNotFoundException e) {
            return null;
        }
    }

    public Manifest getManifest() throws IOException {
        loadJarInputStream();
        return isInputStream ? jis.getManifest() : jarFile.getManifest();
    }

    public InputStream getInputStream(ZipEntry ze) throws IOException {
        loadJarInputStream();
        return isInputStream ? ((JarMapEntry) bufMap.get(ze.getName())).getInputStream() :
                jarFile.getInputStream(ze);
    }

    public OutputStream getOutputStream(ZipEntry ze) {
        if (!isInputStream) // Only support inputstream mode
            return null;
        loadJarInputStream();
        ByteArrayStream bs = ((JarMapEntry) bufMap.get(ze.getName())).data;
        bs.reset();
        return bs;
    }

    public byte[] getRawData(ZipEntry ze) throws IOException {
        if (isInputStream) {
            loadJarInputStream();
            return ((JarMapEntry) bufMap.get(ze.getName())).data.toByteArray();
        } else {
            ByteArrayStream bytes = new ByteArrayStream();
            bytes.readFrom(jarFile.getInputStream(ze));
            return bytes.toByteArray();
        }
    }

    public Enumeration<JarEntry> entries() {
        loadJarInputStream();
        return isInputStream ? Collections.enumeration(bufMap.values()) : jarFile.entries();
    }

    public ZipEntry getEntry(String name) {
        return getJarEntry(name);
    }

    public JarEntry getJarEntry(String name) {
        loadJarInputStream();
        return isInputStream ? bufMap.get(name) : jarFile.getJarEntry(name);
    }

    @Override
    public void close() throws IOException {
        if (isInputStream)
            is.close();
        else
            jarFile.close();
    }

    private static class JarMapEntry extends JarEntry {
        ByteArrayStream data;
        JarMapEntry(JarEntry je, InputStream is) {
            super(je);
            data = new ByteArrayStream();
            data.readFrom(is);
        }
        InputStream getInputStream() {
            return data.getInputStream();
        }
    }
}
