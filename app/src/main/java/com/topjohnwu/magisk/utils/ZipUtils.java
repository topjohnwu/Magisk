package com.topjohnwu.magisk.utils;

import android.content.Context;
import android.text.TextUtils;

import org.bouncycastle.asn1.ASN1InputStream;
import org.bouncycastle.asn1.ASN1ObjectIdentifier;
import org.bouncycastle.asn1.DEROutputStream;
import org.bouncycastle.asn1.cms.CMSObjectIdentifiers;
import org.bouncycastle.asn1.pkcs.PrivateKeyInfo;
import org.bouncycastle.cert.jcajce.JcaCertStore;
import org.bouncycastle.cms.CMSException;
import org.bouncycastle.cms.CMSProcessableByteArray;
import org.bouncycastle.cms.CMSSignedData;
import org.bouncycastle.cms.CMSSignedDataGenerator;
import org.bouncycastle.cms.CMSTypedData;
import org.bouncycastle.cms.jcajce.JcaSignerInfoGeneratorBuilder;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.operator.ContentSigner;
import org.bouncycastle.operator.OperatorCreationException;
import org.bouncycastle.operator.jcajce.JcaContentSignerBuilder;
import org.bouncycastle.operator.jcajce.JcaDigestCalculatorProviderBuilder;
import org.bouncycastle.util.encoders.Base64;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FilterOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.security.DigestOutputStream;
import java.security.GeneralSecurityException;
import java.security.KeyFactory;
import java.security.MessageDigest;
import java.security.PrivateKey;
import java.security.Provider;
import java.security.Security;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Enumeration;
import java.util.Locale;
import java.util.Map;
import java.util.TreeMap;
import java.util.jar.Attributes;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.JarInputStream;
import java.util.jar.JarOutputStream;
import java.util.jar.Manifest;
import java.util.regex.Pattern;

/*
* Modified from from AOSP(Marshmallow) SignAPK.java
* */

public class ZipUtils {
    // File name in assets
    private static final String PUBLIC_KEY_NAME = "public.certificate.x509.pem";
    private static final String PRIVATE_KEY_NAME = "private.key.pk8";
    private static final String UNHIDE_APK = "unhide.apk";

    private static final String CERT_SF_NAME = "META-INF/CERT.SF";
    private static final String CERT_SIG_NAME = "META-INF/CERT.%s";

    private static final String ANDROID_MANIFEST = "AndroidManifest.xml";
    private static final byte[] UNHIDE_PKG_NAME = "com.topjohnwu.unhide\0".getBytes();

    private static Provider sBouncyCastleProvider;
    // bitmasks for which hash algorithms we need the manifest to include.
    private static final int USE_SHA1 = 1;
    private static final int USE_SHA256 = 2;

    static {
        sBouncyCastleProvider = new BouncyCastleProvider();
        Security.insertProviderAt(sBouncyCastleProvider, 1);
        System.loadLibrary("zipadjust");
    }

    public native static void zipAdjust(String filenameIn, String filenameOut);

    public static String generateUnhide(Context context, File output) {
        File temp = new File(context.getCacheDir(), "temp.apk");
        String pkg = "";
        try {
            JarInputStream source = new JarInputStream(context.getAssets().open(UNHIDE_APK));
            JarOutputStream dest = new JarOutputStream(new FileOutputStream(temp));
            JarEntry entry;
            int size;
            byte buffer[] = new byte[4096];
            while ((entry = source.getNextJarEntry()) != null) {
                dest.putNextEntry(new JarEntry(entry.getName()));
                if (TextUtils.equals(entry.getName(), ANDROID_MANIFEST)) {
                    ByteArrayOutputStream baos = new ByteArrayOutputStream();
                    while((size = source.read(buffer)) != -1) {
                        baos.write(buffer, 0, size);
                    }
                    int offset = -1;
                    byte xml[] = baos.toByteArray();

                    // Linear search pattern offset
                    for (int i = 0; i < xml.length - UNHIDE_PKG_NAME.length; ++i) {
                        boolean match = true;
                        for (int j = 0; j < UNHIDE_PKG_NAME.length; ++j) {
                            if (xml[i + j] != UNHIDE_PKG_NAME[j]) {
                                match = false;
                                break;
                            }
                        }
                        if (match) {
                            offset = i;
                            break;
                        }
                    }
                    if (offset < 0)
                        return "";

                    // Patch binary XML with new package name
                    pkg = Utils.genPackageName("com.", UNHIDE_PKG_NAME.length - 1);
                    System.arraycopy(pkg.getBytes(), 0, xml, offset, pkg.length());
                    dest.write(xml);
                } else {
                    while((size = source.read(buffer)) != -1) {
                        dest.write(buffer, 0, size);
                    }
                }
            }
            source.close();
            dest.close();
            signZip(context, temp, output, false);
            temp.delete();
        } catch (IOException e) {
            e.printStackTrace();
            return pkg;
        }
        return pkg;
    }

    public static void removeTopFolder(InputStream in, File output) throws IOException {
        try {
            JarInputStream source = new JarInputStream(in);
            JarOutputStream dest = new JarOutputStream(new BufferedOutputStream(new FileOutputStream(output)));
            JarEntry entry;
            String path;
            int size;
            byte buffer[] = new byte[4096];
            while ((entry = source.getNextJarEntry()) != null) {
                // Remove the top directory from the path
                path = entry.getName().substring(entry.getName().indexOf("/") + 1);
                // If it's the top folder, ignore it
                if (path.isEmpty()) {
                    continue;
                }
                // Don't include placeholder
                if (path.equals("system/placeholder")) {
                    continue;
                }
                dest.putNextEntry(new JarEntry(path));
                while((size = source.read(buffer)) != -1) {
                    dest.write(buffer, 0, size);
                }
            }
            source.close();
            dest.close();
            in.close();
        } catch (IOException e) {
            Logger.dev("ZipUtils: removeTopFolder IO error!");
            throw e;
        }
    }

    public static void unzip(File zip, File folder, String path, boolean junkPath) throws Exception {
        InputStream in = new BufferedInputStream(new FileInputStream(zip));
        unzip(in, folder, path, junkPath);
        in.close();
    }

    public static void unzip(InputStream zip, File folder, String path, boolean junkPath) throws Exception {
        byte data[] = new byte[4096];
        try {
            JarInputStream zipfile = new JarInputStream(zip);
            JarEntry entry;
            while ((entry = zipfile.getNextJarEntry()) != null) {
                if (!entry.getName().startsWith(path) || entry.isDirectory()){
                    // Ignore directories, only create files
                    continue;
                }
                String name;
                if (junkPath) {
                    name = entry.getName().substring(entry.getName().lastIndexOf('/') + 1);
                } else {
                    name = entry.getName();
                }
                Logger.dev("ZipUtils: Extracting: " + entry);
                File dest = new File(folder, name);
                dest.getParentFile().mkdirs();
                FileOutputStream out = new FileOutputStream(dest);
                int count;
                while ((count = zipfile.read(data)) != -1) {
                    out.write(data, 0, count);
                }
                out.flush();
                out.close();
            }
        } catch(Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    public static void signZip(Context context, File input, File output, boolean minSign) {
        int alignment = 4;
        JarFile inputJar = null;
        BufferedOutputStream outputFile = null;
        int hashes = 0;
        try {
            X509Certificate publicKey = readPublicKey(context.getAssets().open(PUBLIC_KEY_NAME));
            hashes |= getDigestAlgorithm(publicKey);

            // Set the ZIP file timestamp to the starting valid time
            // of the 0th certificate plus one hour (to match what
            // we've historically done).
            long timestamp = publicKey.getNotBefore().getTime() + 3600L * 1000;
            PrivateKey privateKey = readPrivateKey(context.getAssets().open(PRIVATE_KEY_NAME));

            outputFile = new BufferedOutputStream(new FileOutputStream(output));
            if (minSign) {
                ZipUtils.signWholeFile(input, publicKey, privateKey, outputFile);
            } else {
                inputJar = new JarFile(input, false);  // Don't verify.
                JarOutputStream outputJar = new JarOutputStream(outputFile);
                // For signing .apks, use the maximum compression to make
                // them as small as possible (since they live forever on
                // the system partition).  For OTA packages, use the
                // default compression level, which is much much faster
                // and produces output that is only a tiny bit larger
                // (~0.1% on full OTA packages I tested).
                outputJar.setLevel(9);
                Manifest manifest = addDigestsToManifest(inputJar, hashes);
                copyFiles(manifest, inputJar, outputJar, timestamp, alignment);
                signFile(manifest, inputJar, publicKey, privateKey, outputJar);
                outputJar.close();
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                if (inputJar != null) inputJar.close();
                if (outputFile != null) outputFile.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }


    /**
     * Return one of USE_SHA1 or USE_SHA256 according to the signature
     * algorithm specified in the cert.
     */
    private static int getDigestAlgorithm(X509Certificate cert) {
        String sigAlg = cert.getSigAlgName().toUpperCase(Locale.US);
        if ("SHA1WITHRSA".equals(sigAlg) ||
                "MD5WITHRSA".equals(sigAlg)) {     // see "HISTORICAL NOTE" above.
            return USE_SHA1;
        } else if (sigAlg.startsWith("SHA256WITH")) {
            return USE_SHA256;
        } else {
            throw new IllegalArgumentException("unsupported signature algorithm \"" + sigAlg +
                    "\" in cert [" + cert.getSubjectDN());
        }
    }
    /** Returns the expected signature algorithm for this key type. */
    private static String getSignatureAlgorithm(X509Certificate cert) {
        String sigAlg = cert.getSigAlgName().toUpperCase(Locale.US);
        String keyType = cert.getPublicKey().getAlgorithm().toUpperCase(Locale.US);
        if ("RSA".equalsIgnoreCase(keyType)) {
            if (getDigestAlgorithm(cert) == USE_SHA256) {
                return "SHA256withRSA";
            } else {
                return "SHA1withRSA";
            }
        } else if ("EC".equalsIgnoreCase(keyType)) {
            return "SHA256withECDSA";
        } else {
            throw new IllegalArgumentException("unsupported key type: " + keyType);
        }
    }
    // Files matching this pattern are not copied to the output.
    private static Pattern stripPattern =
            Pattern.compile("^(META-INF/((.*)[.](SF|RSA|DSA|EC)|com/android/otacert))|(" +
                    Pattern.quote(JarFile.MANIFEST_NAME) + ")$");
    private static X509Certificate readPublicKey(InputStream input)
            throws IOException, GeneralSecurityException {
        try {
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            return (X509Certificate) cf.generateCertificate(input);
        } finally {
            input.close();
        }
    }

    /** Read a PKCS#8 format private key. */
    private static PrivateKey readPrivateKey(InputStream input)
            throws IOException, GeneralSecurityException {
        try {
            byte[] buffer = new byte[4096];
            int size = input.read(buffer);
            byte[] bytes = Arrays.copyOf(buffer, size);
            /* Check to see if this is in an EncryptedPrivateKeyInfo structure. */
            PKCS8EncodedKeySpec spec = new PKCS8EncodedKeySpec(bytes);
            /*
             * Now it's in a PKCS#8 PrivateKeyInfo structure. Read its Algorithm
             * OID and use that to construct a KeyFactory.
             */
            ASN1InputStream bIn = new ASN1InputStream(new ByteArrayInputStream(spec.getEncoded()));
            PrivateKeyInfo pki = PrivateKeyInfo.getInstance(bIn.readObject());
            String algOid = pki.getPrivateKeyAlgorithm().getAlgorithm().getId();
            return KeyFactory.getInstance(algOid).generatePrivate(spec);
        } finally {
            input.close();
        }
    }
    /**
     * Add the hash(es) of every file to the manifest, creating it if
     * necessary.
     */
    private static Manifest addDigestsToManifest(JarFile jar, int hashes)
            throws IOException, GeneralSecurityException {
        Manifest input = jar.getManifest();
        Manifest output = new Manifest();
        Attributes main = output.getMainAttributes();
        if (input != null) {
            main.putAll(input.getMainAttributes());
        } else {
            main.putValue("Manifest-Version", "1.0");
            main.putValue("Created-By", "1.0 (Android SignApk)");
        }
        MessageDigest md_sha1 = null;
        MessageDigest md_sha256 = null;
        if ((hashes & USE_SHA1) != 0) {
            md_sha1 = MessageDigest.getInstance("SHA1");
        }
        if ((hashes & USE_SHA256) != 0) {
            md_sha256 = MessageDigest.getInstance("SHA256");
        }
        byte[] buffer = new byte[4096];
        int num;
        // We sort the input entries by name, and add them to the
        // output manifest in sorted order.  We expect that the output
        // map will be deterministic.
        TreeMap<String, JarEntry> byName = new TreeMap<String, JarEntry>();
        for (Enumeration<JarEntry> e = jar.entries(); e.hasMoreElements(); ) {
            JarEntry entry = e.nextElement();
            byName.put(entry.getName(), entry);
        }
        for (JarEntry entry: byName.values()) {
            String name = entry.getName();
            if (!entry.isDirectory() &&
                    (stripPattern == null || !stripPattern.matcher(name).matches())) {
                InputStream data = jar.getInputStream(entry);
                while ((num = data.read(buffer)) > 0) {
                    if (md_sha1 != null) md_sha1.update(buffer, 0, num);
                    if (md_sha256 != null) md_sha256.update(buffer, 0, num);
                }
                Attributes attr = null;
                if (input != null) attr = input.getAttributes(name);
                attr = attr != null ? new Attributes(attr) : new Attributes();
                if (md_sha1 != null) {
                    attr.putValue("SHA1-Digest",
                            new String(Base64.encode(md_sha1.digest()), "ASCII"));
                }
                if (md_sha256 != null) {
                    attr.putValue("SHA-256-Digest",
                            new String(Base64.encode(md_sha256.digest()), "ASCII"));
                }
                output.getEntries().put(name, attr);
            }
        }
        return output;
    }

    /** Write to another stream and track how many bytes have been
     *  written.
     */
    private static class CountOutputStream extends FilterOutputStream {
        private int mCount;
        public CountOutputStream(OutputStream out) {
            super(out);
            mCount = 0;
        }
        @Override
        public void write(int b) throws IOException {
            super.write(b);
            mCount++;
        }
        @Override
        public void write(byte[] b, int off, int len) throws IOException {
            super.write(b, off, len);
            mCount += len;
        }
        public int size() {
            return mCount;
        }
    }
    /** Write a .SF file with a digest of the specified manifest. */
    private static void writeSignatureFile(Manifest manifest, OutputStream out,
                                           int hash)
            throws IOException, GeneralSecurityException {
        Manifest sf = new Manifest();
        Attributes main = sf.getMainAttributes();
        main.putValue("Signature-Version", "1.0");
        main.putValue("Created-By", "1.0 (Android SignApk)");
        MessageDigest md = MessageDigest.getInstance(
                hash == USE_SHA256 ? "SHA256" : "SHA1");
        PrintStream print = new PrintStream(
                new DigestOutputStream(new ByteArrayOutputStream(), md),
                true, "UTF-8");
        // Digest of the entire manifest
        manifest.write(print);
        print.flush();
        main.putValue(hash == USE_SHA256 ? "SHA-256-Digest-Manifest" : "SHA1-Digest-Manifest",
                new String(Base64.encode(md.digest()), "ASCII"));
        Map<String, Attributes> entries = manifest.getEntries();
        for (Map.Entry<String, Attributes> entry : entries.entrySet()) {
            // Digest of the manifest stanza for this entry.
            print.print("Name: " + entry.getKey() + "\r\n");
            for (Map.Entry<Object, Object> att : entry.getValue().entrySet()) {
                print.print(att.getKey() + ": " + att.getValue() + "\r\n");
            }
            print.print("\r\n");
            print.flush();
            Attributes sfAttr = new Attributes();
            sfAttr.putValue(hash == USE_SHA256 ? "SHA-256-Digest" : "SHA1-Digest-Manifest",
                    new String(Base64.encode(md.digest()), "ASCII"));
            sf.getEntries().put(entry.getKey(), sfAttr);
        }
        CountOutputStream cout = new CountOutputStream(out);
        sf.write(cout);
        // A bug in the java.util.jar implementation of Android platforms
        // up to version 1.6 will cause a spurious IOException to be thrown
        // if the length of the signature file is a multiple of 1024 bytes.
        // As a workaround, add an extra CRLF in this case.
        if ((cout.size() % 1024) == 0) {
            cout.write('\r');
            cout.write('\n');
        }
    }
    /** Sign data and write the digital signature to 'out'. */
    private static void writeSignatureBlock(
            CMSTypedData data, X509Certificate publicKey, PrivateKey privateKey,
            OutputStream out)
            throws IOException,
            CertificateEncodingException,
            OperatorCreationException,
            CMSException {
        ArrayList<X509Certificate> certList = new ArrayList<>(1);
        certList.add(publicKey);
        JcaCertStore certs = new JcaCertStore(certList);
        CMSSignedDataGenerator gen = new CMSSignedDataGenerator();
        ContentSigner signer = new JcaContentSignerBuilder(getSignatureAlgorithm(publicKey))
                .setProvider(sBouncyCastleProvider)
                .build(privateKey);
        gen.addSignerInfoGenerator(
                new JcaSignerInfoGeneratorBuilder(
                        new JcaDigestCalculatorProviderBuilder()
                                .setProvider(sBouncyCastleProvider)
                                .build())
                        .setDirectSignature(true)
                        .build(signer, publicKey));
        gen.addCertificates(certs);
        CMSSignedData sigData = gen.generate(data, false);
        ASN1InputStream asn1 = new ASN1InputStream(sigData.getEncoded());
        DEROutputStream dos = new DEROutputStream(out);
        dos.writeObject(asn1.readObject());
    }
    /**
     * Copy all the files in a manifest from input to output.  We set
     * the modification times in the output to a fixed time, so as to
     * reduce variation in the output file and make incremental OTAs
     * more efficient.
     */
    private static void copyFiles(Manifest manifest, JarFile in, JarOutputStream out,
                                  long timestamp, int alignment) throws IOException {
        byte[] buffer = new byte[4096];
        int num;
        Map<String, Attributes> entries = manifest.getEntries();
        ArrayList<String> names = new ArrayList<String>(entries.keySet());
        Collections.sort(names);
        boolean firstEntry = true;
        long offset = 0L;
        // We do the copy in two passes -- first copying all the
        // entries that are STORED, then copying all the entries that
        // have any other compression flag (which in practice means
        // DEFLATED).  This groups all the stored entries together at
        // the start of the file and makes it easier to do alignment
        // on them (since only stored entries are aligned).
        for (String name : names) {
            JarEntry inEntry = in.getJarEntry(name);
            JarEntry outEntry = null;
            if (inEntry.getMethod() != JarEntry.STORED) continue;
            // Preserve the STORED method of the input entry.
            outEntry = new JarEntry(inEntry);
            outEntry.setTime(timestamp);
            // 'offset' is the offset into the file at which we expect
            // the file data to begin.  This is the value we need to
            // make a multiple of 'alignement'.
            offset += JarFile.LOCHDR + outEntry.getName().length();
            if (firstEntry) {
                // The first entry in a jar file has an extra field of
                // four bytes that you can't get rid of; any extra
                // data you specify in the JarEntry is appended to
                // these forced four bytes.  This is JAR_MAGIC in
                // JarOutputStream; the bytes are 0xfeca0000.
                offset += 4;
                firstEntry = false;
            }
            if (alignment > 0 && (offset % alignment != 0)) {
                // Set the "extra data" of the entry to between 1 and
                // alignment-1 bytes, to make the file data begin at
                // an aligned offset.
                int needed = alignment - (int)(offset % alignment);
                outEntry.setExtra(new byte[needed]);
                offset += needed;
            }
            out.putNextEntry(outEntry);
            InputStream data = in.getInputStream(inEntry);
            while ((num = data.read(buffer)) > 0) {
                out.write(buffer, 0, num);
                offset += num;
            }
            out.flush();
        }
        // Copy all the non-STORED entries.  We don't attempt to
        // maintain the 'offset' variable past this point; we don't do
        // alignment on these entries.
        for (String name : names) {
            JarEntry inEntry = in.getJarEntry(name);
            JarEntry outEntry = null;
            if (inEntry.getMethod() == JarEntry.STORED) continue;
            // Create a new entry so that the compressed len is recomputed.
            outEntry = new JarEntry(name);
            outEntry.setTime(timestamp);
            out.putNextEntry(outEntry);
            InputStream data = in.getInputStream(inEntry);
            while ((num = data.read(buffer)) > 0) {
                out.write(buffer, 0, num);
            }
            out.flush();
        }
    }

    // This class is to provide a file's content, but trimming out the last two bytes
    // Used for signWholeFile
    private static class CMSProcessableFile implements CMSTypedData {

        private File file;
        private ASN1ObjectIdentifier type;
        private byte[] buffer;
        int bufferSize = 0;

        CMSProcessableFile(File file) {
            this.file = file;
            type = new ASN1ObjectIdentifier(CMSObjectIdentifiers.data.getId());
            buffer = new byte[4096];
        }

        @Override
        public ASN1ObjectIdentifier getContentType() {
            return type;
        }

        @Override
        public void write(OutputStream out) throws IOException, CMSException {
            FileInputStream input = new FileInputStream(file);
            long len = file.length() - 2;
            while ((bufferSize = input.read(buffer)) > 0) {
                if (len <= bufferSize) {
                    out.write(buffer, 0, (int) len);
                    break;
                } else {
                    out.write(buffer, 0, bufferSize);
                }
                len -= bufferSize;
            }
        }

        @Override
        public Object getContent() {
            return file;
        }

        byte[] getTail() {
            return Arrays.copyOfRange(buffer, 0, bufferSize);
        }
    }

    private static void signWholeFile(File input, X509Certificate publicKey,
                                      PrivateKey privateKey, OutputStream outputStream)
            throws Exception {
        ByteArrayOutputStream temp = new ByteArrayOutputStream();
        // put a readable message and a null char at the start of the
        // archive comment, so that tools that display the comment
        // (hopefully) show something sensible.
        // TODO: anything more useful we can put in this message?
        byte[] message = "signed by SignApk".getBytes("UTF-8");
        temp.write(message);
        temp.write(0);

        CMSProcessableFile cmsFile = new CMSProcessableFile(input);
        writeSignatureBlock(cmsFile, publicKey, privateKey, temp);

        // For a zip with no archive comment, the
        // end-of-central-directory record will be 22 bytes long, so
        // we expect to find the EOCD marker 22 bytes from the end.
        byte[] zipData = cmsFile.getTail();
        if (zipData[zipData.length-22] != 0x50 ||
                zipData[zipData.length-21] != 0x4b ||
                zipData[zipData.length-20] != 0x05 ||
                zipData[zipData.length-19] != 0x06) {
            throw new IllegalArgumentException("zip data already has an archive comment");
        }
        int total_size = temp.size() + 6;
        if (total_size > 0xffff) {
            throw new IllegalArgumentException("signature is too big for ZIP file comment");
        }
        // signature starts this many bytes from the end of the file
        int signature_start = total_size - message.length - 1;
        temp.write(signature_start & 0xff);
        temp.write((signature_start >> 8) & 0xff);
        // Why the 0xff bytes?  In a zip file with no archive comment,
        // bytes [-6:-2] of the file are the little-endian offset from
        // the start of the file to the central directory.  So for the
        // two high bytes to be 0xff 0xff, the archive would have to
        // be nearly 4GB in size.  So it's unlikely that a real
        // commentless archive would have 0xffs here, and lets us tell
        // an old signed archive from a new one.
        temp.write(0xff);
        temp.write(0xff);
        temp.write(total_size & 0xff);
        temp.write((total_size >> 8) & 0xff);
        temp.flush();
        // Signature verification checks that the EOCD header is the
        // last such sequence in the file (to avoid minzip finding a
        // fake EOCD appended after the signature in its scan).  The
        // odds of producing this sequence by chance are very low, but
        // let's catch it here if it does.
        byte[] b = temp.toByteArray();
        for (int i = 0; i < b.length-3; ++i) {
            if (b[i] == 0x50 && b[i+1] == 0x4b && b[i+2] == 0x05 && b[i+3] == 0x06) {
                throw new IllegalArgumentException("found spurious EOCD header at " + i);
            }
        }
        cmsFile.write(outputStream);
        outputStream.write(total_size & 0xff);
        outputStream.write((total_size >> 8) & 0xff);
        temp.writeTo(outputStream);
    }
    private static void signFile(Manifest manifest, JarFile inputJar,
                                 X509Certificate publicKey, PrivateKey privateKey,
                                 JarOutputStream outputJar)
            throws Exception {
        // Assume the certificate is valid for at least an hour.
        long timestamp = publicKey.getNotBefore().getTime() + 3600L * 1000;
        // MANIFEST.MF
        JarEntry je = new JarEntry(JarFile.MANIFEST_NAME);
        je.setTime(timestamp);
        outputJar.putNextEntry(je);
        manifest.write(outputJar);
        je = new JarEntry(CERT_SF_NAME);
        je.setTime(timestamp);
        outputJar.putNextEntry(je);
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        writeSignatureFile(manifest, baos, getDigestAlgorithm(publicKey));
        byte[] signedData = baos.toByteArray();
        outputJar.write(signedData);
        // CERT.{EC,RSA} / CERT#.{EC,RSA}
        final String keyType = publicKey.getPublicKey().getAlgorithm();
        je = new JarEntry(String.format(CERT_SIG_NAME, keyType));
        je.setTime(timestamp);
        outputJar.putNextEntry(je);
        writeSignatureBlock(new CMSProcessableByteArray(signedData),
                publicKey, privateKey, outputJar);
    }
}