package com.topjohnwu.magisk.utils;

import android.content.Context;
import android.util.Pair;

import org.spongycastle.asn1.ASN1InputStream;
import org.spongycastle.asn1.ASN1ObjectIdentifier;
import org.spongycastle.asn1.DEROutputStream;
import org.spongycastle.asn1.cms.CMSObjectIdentifiers;
import org.spongycastle.cert.jcajce.JcaCertStore;
import org.spongycastle.cms.CMSException;
import org.spongycastle.cms.CMSProcessableByteArray;
import org.spongycastle.cms.CMSSignedData;
import org.spongycastle.cms.CMSSignedDataGenerator;
import org.spongycastle.cms.CMSTypedData;
import org.spongycastle.cms.jcajce.JcaSignerInfoGeneratorBuilder;
import org.spongycastle.jce.provider.BouncyCastleProvider;
import org.spongycastle.operator.ContentSigner;
import org.spongycastle.operator.OperatorCreationException;
import org.spongycastle.operator.jcajce.JcaContentSignerBuilder;
import org.spongycastle.operator.jcajce.JcaDigestCalculatorProviderBuilder;
import org.spongycastle.util.encoders.Base64;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FilterOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.security.DigestOutputStream;
import java.security.GeneralSecurityException;
import java.security.Key;
import java.security.KeyFactory;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.Provider;
import java.security.Security;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
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

import javax.crypto.Cipher;
import javax.crypto.EncryptedPrivateKeyInfo;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class ZipUtils {
    private static final String CERT_SF_NAME = "META-INF/CERT.SF";
    private static final String CERT_SIG_NAME = "META-INF/CERT.%s";
    private static final String OTACERT_NAME = "META-INF/com/android/otacert";
    private static final String PUBLIC_KEY_NAME = "public.certificate.x509.pem";
    private static final String PRIVATE_KEY_NAME = "private.key.pk8";
    private static Provider sBouncyCastleProvider;
    // bitmasks for which hash algorithms we need the manifest to include.
    private static final int USE_SHA1 = 1;
    private static final int USE_SHA256 = 2;

    static {
        System.loadLibrary("zipadjust");
        sBouncyCastleProvider = new BouncyCastleProvider();
        Security.insertProviderAt(sBouncyCastleProvider, 1);
    }

    public native static byte[] zipAdjust(byte[] bytes, int size);

    // Wrapper function for the JNI function
    public static void adjustZip(ByteArrayInOutStream buffer) {
        buffer.setBuffer(zipAdjust(buffer.toByteArray(), buffer.size()));
    }

    public static void removeTopFolder(InputStream in, OutputStream out) {
        try {
            JarInputStream source = new JarInputStream(in);
            JarOutputStream dest = new JarOutputStream(out);
            JarEntry entry;
            String path;
            int size;
            byte buffer[] = new byte[4096];
            while ((entry = source.getNextJarEntry()) != null) {
                // Remove the top directory from the path
                path = entry.getName().substring(entry.getName().indexOf("/") + 1);
                // If it's the top folder, ignore it
                if (path.isEmpty())
                    continue;
                // Don't include placeholder
                if (path.contains("system/placeholder"))
                    continue;
                dest.putNextEntry(new JarEntry(path));
                while((size = source.read(buffer, 0, 2048)) != -1)
                    dest.write(buffer, 0, size);
            }
            source.close();
            dest.close();
            in.close();
        } catch (IOException e) {
            e.printStackTrace();
            Logger.dev("ZipUtils: removeTopFolder IO error!");
        }
    }

    public static void unzip(File file, File folder) {
        unzip(file, folder, "");
    }

    public static void unzip(File file, File folder, String path) {
        int count;
        FileOutputStream out;
        File dest;
        InputStream is;
        JarEntry entry;
        byte data[] = new byte[4096];
        try (JarFile zipfile = new JarFile(file)) {
            Enumeration<JarEntry> e = zipfile.entries();
            while(e.hasMoreElements()) {
                entry = e.nextElement();
                if (!entry.getName().contains(path)
                        || entry.getName().charAt(entry.getName().length() - 1) == '/') {
                    // Ignore directories, only create files
                    continue;
                }
                Logger.dev("ZipUtils: Extracting: " + entry);
                is = zipfile.getInputStream(entry);
                dest = new File(folder, entry.getName());
                if (dest.getParentFile().mkdirs()) {
                    dest.createNewFile();
                }
                out = new FileOutputStream(dest);
                while ((count = is.read(data, 0, 4096)) != -1) {
                    out.write(data, 0, count);
                }
                out.flush();
                out.close();
                is.close();
            }
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    public static void signZip(Context context, InputStream inputStream,
                               OutputStream outputStream, boolean signWholeFile) {
        JarMap inputJar;
        int hashes = 0;
        try {
            X509Certificate publicKey = readPublicKey(context.getAssets().open(PUBLIC_KEY_NAME));
            hashes |= getDigestAlgorithm(publicKey);
            // Set the ZIP file timestamp to the starting valid time
            // of the 0th certificate plus one hour (to match what
            // we've historically done).
            long timestamp = publicKey.getNotBefore().getTime() + 3600L * 1000;
            PrivateKey privateKey = readPrivateKey(context.getAssets().open(PRIVATE_KEY_NAME));
            inputJar = new JarMap(new JarInputStream(inputStream));
            if (signWholeFile) {
                if (!"RSA".equalsIgnoreCase(privateKey.getAlgorithm())) {
                    throw new IOException("Cannot sign OTA packages with non-RSA keys");
                }
                signWholeFile(inputJar, context.getAssets().open(PUBLIC_KEY_NAME),
                        publicKey, privateKey, outputStream);
            } else {
                JarOutputStream outputJar = new JarOutputStream(outputStream);
                // For signing .apks, use the maximum compression to make
                // them as small as possible (since they live forever on
                // the system partition).  For OTA packages, use the
                // default compression level, which is much much faster
                // and produces output that is only a tiny bit larger
                // (~0.1% on full OTA packages I tested).
                outputJar.setLevel(9);
                Manifest manifest = addDigestsToManifest(inputJar, hashes);
                copyFiles(manifest, inputJar, outputJar, timestamp);
                signFile(manifest, inputJar, publicKey, privateKey, outputJar);
                outputJar.close();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static class JarMap extends TreeMap<String, Pair<JarEntry, ByteArrayOutputStream> > {

        private Manifest manifest;

        public JarMap(JarInputStream in) throws IOException {
            super();
            manifest = in.getManifest();
            byte[] buffer = new byte[4096];
            int num;
            JarEntry entry;
            while ((entry = in.getNextJarEntry()) != null) {
                ByteArrayOutputStream stream = new ByteArrayOutputStream();
                while ((num = in.read(buffer)) > 0) {
                    stream.write(buffer, 0, num);
                }
                put(entry.getName(), entry, stream);
            }
            in.close();
        }

        public JarEntry getJarEntry(String name) {
            return get(name).first;
        }
        public ByteArrayOutputStream getStream(String name) {
            return get(name).second;
        }
        public void put(String name, JarEntry entry, ByteArrayOutputStream stream) {
            put(name, new Pair<>(entry, stream));
        }
        public Manifest getManifest() {
            return manifest;
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
        } else if ("DSA".equalsIgnoreCase(keyType)) {
            return "SHA256withDSA";
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

    /**
     * Decrypt an encrypted PKCS 8 format private key.
     *
     * Based on ghstark's post on Aug 6, 2006 at
     * http://forums.sun.com/thread.jspa?threadID=758133&messageID=4330949
     *
     * @param encryptedPrivateKey The raw data of the private key
     * @param keyFile The file containing the private key
     */
    private static KeySpec decryptPrivateKey(byte[] encryptedPrivateKey, File keyFile)
            throws GeneralSecurityException {
        EncryptedPrivateKeyInfo epkInfo;
        try {
            epkInfo = new EncryptedPrivateKeyInfo(encryptedPrivateKey);
        } catch (IOException ex) {
            // Probably not an encrypted key.
            return null;
        }
        // We no longer have console, so need to use another way to input password
        // This function is left here if needed in the future, so no use for now
        char[] password = new char[0];
        SecretKeyFactory skFactory = SecretKeyFactory.getInstance(epkInfo.getAlgName());
        Key key = skFactory.generateSecret(new PBEKeySpec(password));
        Cipher cipher = Cipher.getInstance(epkInfo.getAlgName());
        cipher.init(Cipher.DECRYPT_MODE, key, epkInfo.getAlgParameters());
        try {
            return epkInfo.getKeySpec(cipher);
        } catch (InvalidKeySpecException ex) {
            System.err.println("signapk: Password for " + keyFile + " may be bad.");
            throw ex;
        }
    }

    /** Read a PKCS 8 format private key. */
    private static PrivateKey readPrivateKey(InputStream input)
            throws IOException, GeneralSecurityException {
        try {
            byte[] buffer = new byte[4096];
            int size = input.read(buffer);
            byte[] bytes = Arrays.copyOf(buffer, size);
            KeySpec spec = new PKCS8EncodedKeySpec(bytes);
            PrivateKey key;
            key = decodeAsKeyType(spec, "RSA");
            if (key != null) {
                return key;
            }
            key = decodeAsKeyType(spec, "DSA");
            if (key != null) {
                return key;
            }
            key = decodeAsKeyType(spec, "EC");
            if (key != null) {
                return key;
            }
            throw new NoSuchAlgorithmException("Must be an RSA, DSA, or EC key");
        } finally {
            input.close();
        }
    }
    private static PrivateKey decodeAsKeyType(KeySpec spec, String keyType)
            throws GeneralSecurityException {
        try {
            return KeyFactory.getInstance(keyType).generatePrivate(spec);
        } catch (InvalidKeySpecException e) {
            return null;
        }
    }

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
        // We sort the input entries by name, and add them to the
        // output manifest in sorted order.  We expect that the output
        // map will be deterministic.
            /* JarMap is a TreeMap, so it's already sorted */
        for (String name : jar.keySet()) {
            JarEntry entry = jar.getJarEntry(name);
            if (!entry.isDirectory() &&
                    (stripPattern == null || !stripPattern.matcher(name).matches())) {
                byte[] buffer = jar.getStream(name).toByteArray();
                if (md_sha1 != null) md_sha1.update(buffer, 0, buffer.length);
                if (md_sha256 != null) md_sha256.update(buffer, 0, buffer.length);
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

    /**
     * Add a copy of the public key to the archive; this should
     * exactly match one of the files in
     * /system/etc/security/otacerts.zip on the device.  (The same
     * cert can be extracted from the CERT.RSA file but this is much
     * easier to get at.)
     */
    private static void addOtacert(JarOutputStream outputJar,
                                   InputStream input,
                                   long timestamp,
                                   Manifest manifest,
                                   int hash)
            throws IOException, GeneralSecurityException {
        MessageDigest md = MessageDigest.getInstance(hash == USE_SHA1 ? "SHA1" : "SHA256");
        JarEntry je = new JarEntry(OTACERT_NAME);
        je.setTime(timestamp);
        outputJar.putNextEntry(je);
        byte[] b = new byte[4096];
        int read;
        while ((read = input.read(b)) != -1) {
            outputJar.write(b, 0, read);
            md.update(b, 0, read);
        }
        input.close();
        Attributes attr = new Attributes();
        attr.putValue(hash == USE_SHA1 ? "SHA1-Digest" : "SHA-256-Digest",
                new String(Base64.encode(md.digest()), "ASCII"));
        manifest.getEntries().put(OTACERT_NAME, attr);
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
            CMSTypedData data, X509Certificate publicKey, PrivateKey privateKey, OutputStream out)
            throws IOException, CertificateEncodingException, OperatorCreationException, CMSException {
        ArrayList<X509Certificate> certList = new ArrayList<X509Certificate>(1);
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
        try (ASN1InputStream asn1 = new ASN1InputStream(sigData.getEncoded())) {
            DEROutputStream dos = new DEROutputStream(out);
            dos.writeObject(asn1.readObject());
        }
    }
    /**
     * Copy all the files in a manifest from input to output.  We set
     * the modification times in the output to a fixed time, so as to
     * reduce variation in the output file and make incremental OTAs
     * more efficient.
     */
    private static void copyFiles(Manifest manifest,
                                  JarMap in, JarOutputStream out, long timestamp) throws IOException {
        Map<String, Attributes> entries = manifest.getEntries();
        ArrayList<String> names = new ArrayList<>(entries.keySet());
        Collections.sort(names);
        for (String name : names) {
            JarEntry inEntry = in.getJarEntry(name);
            JarEntry outEntry;
            if (inEntry.getMethod() == JarEntry.STORED) {
                // Preserve the STORED method of the input entry.
                outEntry = new JarEntry(inEntry);
            } else {
                // Create a new entry so that the compressed len is recomputed.
                outEntry = new JarEntry(name);
            }
            outEntry.setTime(timestamp);
            out.putNextEntry(outEntry);
            in.getStream(name).writeTo(out);
            out.flush();
        }
    }

    private static class WholeFileSignerOutputStream extends FilterOutputStream {
        private boolean closing = false;
        private ByteArrayOutputStream footer = new ByteArrayOutputStream();
        private OutputStream tee;
        public WholeFileSignerOutputStream(OutputStream out, OutputStream tee) {
            super(out);
            this.tee = tee;
        }
        public void notifyClosing() {
            closing = true;
        }
        public void finish() throws IOException {
            closing = false;
            byte[] data = footer.toByteArray();
            if (data.length < 2)
                throw new IOException("Less than two bytes written to footer");
            write(data, 0, data.length - 2);
        }
        public byte[] getTail() {
            return footer.toByteArray();
        }
        @Override
        public void write(byte[] b) throws IOException {
            write(b, 0, b.length);
        }
        @Override
        public void write(byte[] b, int off, int len) throws IOException {
            if (closing) {
                // if the jar is about to close, save the footer that will be written
                footer.write(b, off, len);
            }
            else {
                // write to both output streams. out is the CMSTypedData signer and tee is the file.
                out.write(b, off, len);
                tee.write(b, off, len);
            }
        }
        @Override
        public void write(int b) throws IOException {
            if (closing) {
                // if the jar is about to close, save the footer that will be written
                footer.write(b);
            }
            else {
                // write to both output streams. out is the CMSTypedData signer and tee is the file.
                out.write(b);
                tee.write(b);
            }
        }
    }

    private static class CMSSigner implements CMSTypedData {
        private JarMap inputJar;
        private InputStream publicKeyFile;
        private X509Certificate publicKey;
        private PrivateKey privateKey;
        private OutputStream outputStream;
        private final ASN1ObjectIdentifier type;
        private WholeFileSignerOutputStream signer;
        public CMSSigner(JarMap inputJar, InputStream publicKeyFile,
                         X509Certificate publicKey, PrivateKey privateKey,
                         OutputStream outputStream) {
            this.inputJar = inputJar;
            this.publicKeyFile = publicKeyFile;
            this.publicKey = publicKey;
            this.privateKey = privateKey;
            this.outputStream = outputStream;
            this.type = new ASN1ObjectIdentifier(CMSObjectIdentifiers.data.getId());
        }
        public Object getContent() {
            // Not supported, but still don't crash or return null
            return 1;
        }
        public ASN1ObjectIdentifier getContentType() {
            return type;
        }
        public void write(OutputStream out) throws IOException {
            try {
                signer = new WholeFileSignerOutputStream(out, outputStream);
                JarOutputStream outputJar = new JarOutputStream(signer);
                int hash = getDigestAlgorithm(publicKey);
                // Assume the certificate is valid for at least an hour.
                long timestamp = publicKey.getNotBefore().getTime() + 3600L * 1000;
                Manifest manifest = addDigestsToManifest(inputJar, hash);
                copyFiles(manifest, inputJar, outputJar, timestamp);
                // Don't add Otacert, it's not an OTA
                // addOtacert(outputJar, publicKeyFile, timestamp, manifest, hash);
                signFile(manifest, inputJar, publicKey, privateKey, outputJar);
                signer.notifyClosing();
                outputJar.close();
                signer.finish();
            }
            catch (Exception e) {
                throw new IOException(e);
            }
        }
        public void writeSignatureBlock(ByteArrayOutputStream temp)
                throws IOException,
                CertificateEncodingException,
                OperatorCreationException,
                CMSException {
            ZipUtils.writeSignatureBlock(this, publicKey, privateKey, temp);
        }
        public WholeFileSignerOutputStream getSigner() {
            return signer;
        }
    }

    private static void signWholeFile(JarMap inputJar, InputStream publicKeyFile,
                                      X509Certificate publicKey, PrivateKey privateKey,
                                      OutputStream outputStream) throws Exception {
        CMSSigner cmsOut = new CMSSigner(inputJar, publicKeyFile,
                publicKey, privateKey, outputStream);
        ByteArrayOutputStream temp = new ByteArrayOutputStream();
        // put a readable message and a null char at the start of the
        // archive comment, so that tools that display the comment
        // (hopefully) show something sensible.
        // TODO: anything more useful we can put in this message?
        byte[] message = "signed by SignApk".getBytes("UTF-8");
        temp.write(message);
        temp.write(0);
        cmsOut.writeSignatureBlock(temp);
        byte[] zipData = cmsOut.getSigner().getTail();
        // For a zip with no archive comment, the
        // end-of-central-directory record will be 22 bytes long, so
        // we expect to find the EOCD marker 22 bytes from the end.
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
        outputStream.write(total_size & 0xff);
        outputStream.write((total_size >> 8) & 0xff);
        temp.writeTo(outputStream);
    }

    private static void signFile(Manifest manifest, JarMap inputJar,
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
        // CERT.SF / CERT#.SF
        je = new JarEntry(CERT_SF_NAME);
        je.setTime(timestamp);
        outputJar.putNextEntry(je);
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        writeSignatureFile(manifest, baos, getDigestAlgorithm(publicKey));
        byte[] signedData = baos.toByteArray();
        outputJar.write(signedData);
        // CERT.{DSA,EC,RSA} / CERT#.{DSA,EC,RSA}
        je = new JarEntry((String.format(CERT_SIG_NAME, privateKey.getAlgorithm())));
        je.setTime(timestamp);
        outputJar.putNextEntry(je);
        writeSignatureBlock(new CMSProcessableByteArray(signedData),
                publicKey, privateKey, outputJar);
    }
}
