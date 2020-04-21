package com.topjohnwu.signing;

import org.bouncycastle.asn1.ASN1Encoding;
import org.bouncycastle.asn1.ASN1InputStream;
import org.bouncycastle.asn1.ASN1ObjectIdentifier;
import org.bouncycastle.asn1.ASN1OutputStream;
import org.bouncycastle.asn1.cms.CMSObjectIdentifiers;
import org.bouncycastle.cert.jcajce.JcaCertStore;
import org.bouncycastle.cms.CMSException;
import org.bouncycastle.cms.CMSProcessableByteArray;
import org.bouncycastle.cms.CMSSignedData;
import org.bouncycastle.cms.CMSSignedDataGenerator;
import org.bouncycastle.cms.CMSTypedData;
import org.bouncycastle.cms.jcajce.JcaSignerInfoGeneratorBuilder;
import org.bouncycastle.operator.ContentSigner;
import org.bouncycastle.operator.OperatorCreationException;
import org.bouncycastle.operator.jcajce.JcaContentSignerBuilder;
import org.bouncycastle.operator.jcajce.JcaDigestCalculatorProviderBuilder;
import org.bouncycastle.util.encoders.Base64;

import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FilterOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.RandomAccessFile;
import java.security.DigestOutputStream;
import java.security.GeneralSecurityException;
import java.security.MessageDigest;
import java.security.PrivateKey;
import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.Locale;
import java.util.Map;
import java.util.TreeMap;
import java.util.jar.Attributes;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.JarOutputStream;
import java.util.jar.Manifest;
import java.util.regex.Pattern;

/*
* Modified from from AOSP
* https://android.googlesource.com/platform/build/+/refs/heads/marshmallow-release/tools/signapk/SignApk.java
* */

public class SignAPK {

    private static final String CERT_SF_NAME = "META-INF/CERT.SF";
    private static final String CERT_SIG_NAME = "META-INF/CERT.%s";

    // bitmasks for which hash algorithms we need the manifest to include.
    private static final int USE_SHA1 = 1;
    private static final int USE_SHA256 = 2;

    public static void signAndAdjust(X509Certificate cert, PrivateKey key,
                                     JarMap input, OutputStream output) throws Exception {
        File temp1 = File.createTempFile("signAPK", null);
        File temp2 = File.createTempFile("signAPK", null);

        try {
            try (OutputStream out = new BufferedOutputStream(new FileOutputStream(temp1))) {
                sign(cert, key, input, out, false);
            }

            ZipAdjust.adjust(temp1, temp2);

            try (JarMap map = JarMap.open(temp2, false)) {
                sign(cert, key, map, output, true);
            }
        } finally {
            temp1.delete();
            temp2.delete();
        }
    }

    public static void sign(X509Certificate cert, PrivateKey key,
                            JarMap input, OutputStream output) throws Exception {
        sign(cert, key, input, output, false);
    }

    private static void sign(X509Certificate cert, PrivateKey key, JarMap input,
                             OutputStream output, boolean wholeFile) throws Exception {
        int hashes = 0;
        hashes |= getDigestAlgorithm(cert);

        // Set the ZIP file timestamp to the starting valid time
        // of the 0th certificate plus one hour (to match what
        // we've historically done).
        long timestamp = cert.getNotBefore().getTime() + 3600L * 1000;

        if (wholeFile) {
            signWholeFile(input.getFile(), cert, key, output);
        } else {
            JarOutputStream outputJar = new JarOutputStream(output);
            // For signing .apks, use the maximum compression to make
            // them as small as possible (since they live forever on
            // the system partition).  For OTA packages, use the
            // default compression level, which is much much faster
            // and produces output that is only a tiny bit larger
            // (~0.1% on full OTA packages I tested).
            outputJar.setLevel(9);
            Manifest manifest = addDigestsToManifest(input, hashes);
            copyFiles(manifest, input, outputJar, timestamp, 4);
            signFile(manifest, input, cert, key, outputJar);
            outputJar.close();
        }
    }

    /**
     * Return one of USE_SHA1 or USE_SHA256 according to the signature
     * algorithm specified in the cert.
     */
    private static int getDigestAlgorithm(X509Certificate cert) {
        String sigAlg = cert.getSigAlgName().toUpperCase(Locale.US);
        if (sigAlg.startsWith("SHA1WITHRSA") || sigAlg.startsWith("MD5WITHRSA")) {
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

    /**
     * Add the hash(es) of every file to the manifest, creating it if
     * necessary.
     */
    private static Manifest addDigestsToManifest(JarMap jar, int hashes)
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
        TreeMap<String, JarEntry> byName = new TreeMap<>();
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
                Attributes attr = new Attributes();
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

    /**
     * Write to another stream and track how many bytes have been
     * written.
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

    /**
     * An OutputStream that does literally nothing
     */
    private static OutputStream stubStream = new OutputStream() {
        @Override
        public void write(int b) {}

        @Override
        public void write(byte[] b, int off, int len) {}
    };

    /** Write a .SF file with a digest of the specified manifest. */
    private static void writeSignatureFile(Manifest manifest, OutputStream out, int hash)
            throws IOException, GeneralSecurityException {
        Manifest sf = new Manifest();
        Attributes main = sf.getMainAttributes();
        main.putValue("Signature-Version", "1.0");
        main.putValue("Created-By", "1.0 (Android SignApk)");
        MessageDigest md = MessageDigest.getInstance(
                hash == USE_SHA256 ? "SHA256" : "SHA1");
        PrintStream print = new PrintStream(
                new DigestOutputStream(stubStream, md),
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
            sfAttr.putValue(hash == USE_SHA256 ? "SHA-256-Digest" : "SHA1-Digest",
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
                .build(privateKey);
        gen.addSignerInfoGenerator(
                new JcaSignerInfoGeneratorBuilder(
                        new JcaDigestCalculatorProviderBuilder().build())
                        .setDirectSignature(true)
                        .build(signer, publicKey));
        gen.addCertificates(certs);
        CMSSignedData sigData = gen.generate(data, false);
        ASN1InputStream asn1 = new ASN1InputStream(sigData.getEncoded());
        ASN1OutputStream dos = ASN1OutputStream.create(out, ASN1Encoding.DER);
        dos.writeObject(asn1.readObject());
    }

    /**
     * Copy all the files in a manifest from input to output.  We set
     * the modification times in the output to a fixed time, so as to
     * reduce variation in the output file and make incremental OTAs
     * more efficient.
     */
    private static void copyFiles(Manifest manifest, JarMap in, JarOutputStream out,
                                  long timestamp, int alignment) throws IOException {
        byte[] buffer = new byte[4096];
        int num;
        Map<String, Attributes> entries = manifest.getEntries();
        ArrayList<String> names = new ArrayList<>(entries.keySet());
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

        private ASN1ObjectIdentifier type;
        private RandomAccessFile file;

        CMSProcessableFile(File file) throws FileNotFoundException {
            this.file = new RandomAccessFile(file, "r");
            type = new ASN1ObjectIdentifier(CMSObjectIdentifiers.data.getId());
        }

        @Override
        public ASN1ObjectIdentifier getContentType() {
            return type;
        }

        @Override
        public void write(OutputStream out) throws IOException, CMSException {
            file.seek(0);
            int read;
            byte buffer[] = new byte[4096];
            int len = (int) file.length() - 2;
            while ((read = file.read(buffer, 0, len < buffer.length ? len : buffer.length)) > 0) {
                out.write(buffer, 0, read);
                len -= read;
            }
        }

        @Override
        public Object getContent() {
            return file;
        }

        byte[] getTail() throws IOException {
            byte tail[] = new byte[22];
            file.seek(file.length() - 22);
            file.readFully(tail);
            return tail;
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
        outputStream.close();
    }

    private static void signFile(Manifest manifest, JarMap inputJar,
                                 X509Certificate cert, PrivateKey privateKey,
                                 JarOutputStream outputJar)
            throws Exception {
        // Assume the certificate is valid for at least an hour.
        long timestamp = cert.getNotBefore().getTime() + 3600L * 1000;
        // MANIFEST.MF
        JarEntry je = new JarEntry(JarFile.MANIFEST_NAME);
        je.setTime(timestamp);
        outputJar.putNextEntry(je);
        manifest.write(outputJar);
        je = new JarEntry(CERT_SF_NAME);
        je.setTime(timestamp);
        outputJar.putNextEntry(je);
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        writeSignatureFile(manifest, baos, getDigestAlgorithm(cert));
        byte[] signedData = baos.toByteArray();
        outputJar.write(signedData);
        // CERT.{EC,RSA} / CERT#.{EC,RSA}
        final String keyType = cert.getPublicKey().getAlgorithm();
        je = new JarEntry(String.format(CERT_SIG_NAME, keyType));
        je.setTime(timestamp);
        outputJar.putNextEntry(je);
        writeSignatureBlock(new CMSProcessableByteArray(signedData),
                cert, privateKey, outputJar);
    }
}
