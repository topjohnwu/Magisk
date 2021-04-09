package com.topjohnwu.signing;

import org.bouncycastle.asn1.ASN1Encoding;
import org.bouncycastle.asn1.ASN1InputStream;
import org.bouncycastle.asn1.ASN1OutputStream;
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

import java.io.ByteArrayOutputStream;
import java.io.FileOutputStream;
import java.io.FilterOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.nio.ByteBuffer;
import java.security.DigestOutputStream;
import java.security.GeneralSecurityException;
import java.security.InvalidKeyException;
import java.security.MessageDigest;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.TimeZone;
import java.util.TreeMap;
import java.util.jar.Attributes;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.JarOutputStream;
import java.util.jar.Manifest;
import java.util.regex.Pattern;

/*
 * Modified from from AOSP
 * https://android.googlesource.com/platform/build/+/refs/tags/android-7.1.2_r39/tools/signapk/src/com/android/signapk/SignApk.java
 * */

public class SignApk {
    private static final String CERT_SF_NAME = "META-INF/CERT.SF";
    private static final String CERT_SIG_NAME = "META-INF/CERT.%s";
    private static final String CERT_SF_MULTI_NAME = "META-INF/CERT%d.SF";
    private static final String CERT_SIG_MULTI_NAME = "META-INF/CERT%d.%s";

    // bitmasks for which hash algorithms we need the manifest to include.
    private static final int USE_SHA1 = 1;
    private static final int USE_SHA256 = 2;

    /**
     * Digest algorithm used when signing the APK using APK Signature Scheme v2.
     */
    private static final String APK_SIG_SCHEME_V2_DIGEST_ALGORITHM = "SHA-256";
    // Files matching this pattern are not copied to the output.
    private static final Pattern stripPattern =
            Pattern.compile("^(META-INF/((.*)[.](SF|RSA|DSA|EC)|com/android/otacert))|(" +
                    Pattern.quote(JarFile.MANIFEST_NAME) + ")$");

    /**
     * Return one of USE_SHA1 or USE_SHA256 according to the signature
     * algorithm specified in the cert.
     */
    private static int getDigestAlgorithm(X509Certificate cert) {
        String sigAlg = cert.getSigAlgName().toUpperCase(Locale.US);
        if ("SHA1WITHRSA".equals(sigAlg) || "MD5WITHRSA".equals(sigAlg)) {
            return USE_SHA1;
        } else if (sigAlg.startsWith("SHA256WITH")) {
            return USE_SHA256;
        } else {
            throw new IllegalArgumentException("unsupported signature algorithm \"" + sigAlg +
                    "\" in cert [" + cert.getSubjectDN());
        }
    }

    /**
     * Returns the expected signature algorithm for this key type.
     */
    private static String getSignatureAlgorithm(X509Certificate cert) {
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

        for (JarEntry entry : byName.values()) {
            String name = entry.getName();
            if (!entry.isDirectory() && !stripPattern.matcher(name).matches()) {
                InputStream data = jar.getInputStream(entry);
                while ((num = data.read(buffer)) > 0) {
                    if (md_sha1 != null) md_sha1.update(buffer, 0, num);
                    if (md_sha256 != null) md_sha256.update(buffer, 0, num);
                }

                Attributes attr = null;
                if (input != null) attr = input.getAttributes(name);
                attr = attr != null ? new Attributes(attr) : new Attributes();
                // Remove any previously computed digests from this entry's attributes.
                for (Iterator<Object> i = attr.keySet().iterator(); i.hasNext(); ) {
                    Object key = i.next();
                    if (!(key instanceof Attributes.Name)) {
                        continue;
                    }
                    String attributeNameLowerCase =
                            key.toString().toLowerCase(Locale.US);
                    if (attributeNameLowerCase.endsWith("-digest")) {
                        i.remove();
                    }
                }
                // Add SHA-1 digest if requested
                if (md_sha1 != null) {
                    attr.putValue("SHA1-Digest",
                            new String(Base64.encode(md_sha1.digest()), "ASCII"));
                }
                // Add SHA-256 digest if requested
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
     * Write a .SF file with a digest of the specified manifest.
     */
    private static void writeSignatureFile(Manifest manifest, OutputStream out,
                                           int hash)
            throws IOException, GeneralSecurityException {
        Manifest sf = new Manifest();
        Attributes main = sf.getMainAttributes();
        main.putValue("Signature-Version", "1.0");
        main.putValue("Created-By", "1.0 (Android SignApk)");
        // Add APK Signature Scheme v2 signature stripping protection.
        // This attribute indicates that this APK is supposed to have been signed using one or
        // more APK-specific signature schemes in addition to the standard JAR signature scheme
        // used by this code. APK signature verifier should reject the APK if it does not
        // contain a signature for the signature scheme the verifier prefers out of this set.
        main.putValue(
                ApkSignerV2.SF_ATTRIBUTE_ANDROID_APK_SIGNED_NAME,
                ApkSignerV2.SF_ATTRIBUTE_ANDROID_APK_SIGNED_VALUE);

        MessageDigest md = MessageDigest.getInstance(hash == USE_SHA256 ? "SHA256" : "SHA1");
        PrintStream print = new PrintStream(new DigestOutputStream(new ByteArrayOutputStream(), md),
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

    /**
     * Sign data and write the digital signature to 'out'.
     */
    private static void writeSignatureBlock(
            CMSTypedData data, X509Certificate publicKey, PrivateKey privateKey, OutputStream out)
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
                new JcaSignerInfoGeneratorBuilder(new JcaDigestCalculatorProviderBuilder().build())
                        .setDirectSignature(true)
                        .build(signer, publicKey)
        );
        gen.addCertificates(certs);
        CMSSignedData sigData = gen.generate(data, false);

        try (ASN1InputStream asn1 = new ASN1InputStream(sigData.getEncoded())) {
            ASN1OutputStream dos = ASN1OutputStream.create(out, ASN1Encoding.DER);
            dos.writeObject(asn1.readObject());
        }
    }

    /**
     * Copy all the files in a manifest from input to output.  We set
     * the modification times in the output to a fixed time, so as to
     * reduce variation in the output file and make incremental OTAs
     * more efficient.
     */
    private static void copyFiles(Manifest manifest, JarMap in, JarOutputStream out,
                                  long timestamp, int defaultAlignment) throws IOException {
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
            JarEntry outEntry;
            if (inEntry.getMethod() != JarEntry.STORED) continue;
            // Preserve the STORED method of the input entry.
            outEntry = new JarEntry(inEntry);
            outEntry.setTime(timestamp);
            // Discard comment and extra fields of this entry to
            // simplify alignment logic below and for consistency with
            // how compressed entries are handled later.
            outEntry.setComment(null);
            outEntry.setExtra(null);

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
            int alignment = getStoredEntryDataAlignment(name, defaultAlignment);
            if (alignment > 0 && (offset % alignment != 0)) {
                // Set the "extra data" of the entry to between 1 and
                // alignment-1 bytes, to make the file data begin at
                // an aligned offset.
                int needed = alignment - (int) (offset % alignment);
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
            JarEntry outEntry;
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

    /**
     * Returns the multiple (in bytes) at which the provided {@code STORED} entry's data must start
     * relative to start of file or {@code 0} if alignment of this entry's data is not important.
     */
    private static int getStoredEntryDataAlignment(String entryName, int defaultAlignment) {
        if (defaultAlignment <= 0) {
            return 0;
        }

        if (entryName.endsWith(".so")) {
            // Align .so contents to memory page boundary to enable memory-mapped
            // execution.
            return 4096;
        } else {
            return defaultAlignment;
        }
    }

    private static void signFile(Manifest manifest,
                                 X509Certificate[] publicKey, PrivateKey[] privateKey,
                                 long timestamp, JarOutputStream outputJar) throws Exception {
        // MANIFEST.MF
        JarEntry je = new JarEntry(JarFile.MANIFEST_NAME);
        je.setTime(timestamp);
        outputJar.putNextEntry(je);
        manifest.write(outputJar);

        int numKeys = publicKey.length;
        for (int k = 0; k < numKeys; ++k) {
            // CERT.SF / CERT#.SF
            je = new JarEntry(numKeys == 1 ? CERT_SF_NAME :
                    (String.format(Locale.US, CERT_SF_MULTI_NAME, k)));
            je.setTime(timestamp);
            outputJar.putNextEntry(je);
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            writeSignatureFile(manifest, baos, getDigestAlgorithm(publicKey[k]));
            byte[] signedData = baos.toByteArray();
            outputJar.write(signedData);

            // CERT.{EC,RSA} / CERT#.{EC,RSA}
            final String keyType = publicKey[k].getPublicKey().getAlgorithm();
            je = new JarEntry(numKeys == 1 ? (String.format(CERT_SIG_NAME, keyType)) :
                    (String.format(Locale.US, CERT_SIG_MULTI_NAME, k, keyType)));
            je.setTime(timestamp);
            outputJar.putNextEntry(je);
            writeSignatureBlock(new CMSProcessableByteArray(signedData),
                    publicKey[k], privateKey[k], outputJar);
        }
    }

    /**
     * Converts the provided lists of private keys, their X.509 certificates, and digest algorithms
     * into a list of APK Signature Scheme v2 {@code SignerConfig} instances.
     */
    private static List<ApkSignerV2.SignerConfig> createV2SignerConfigs(
            PrivateKey[] privateKeys, X509Certificate[] certificates, String[] digestAlgorithms)
            throws InvalidKeyException {
        if (privateKeys.length != certificates.length) {
            throw new IllegalArgumentException(
                    "The number of private keys must match the number of certificates: "
                            + privateKeys.length + " vs" + certificates.length);
        }
        List<ApkSignerV2.SignerConfig> result = new ArrayList<>(privateKeys.length);
        for (int i = 0; i < privateKeys.length; i++) {
            PrivateKey privateKey = privateKeys[i];
            X509Certificate certificate = certificates[i];
            PublicKey publicKey = certificate.getPublicKey();
            String keyAlgorithm = privateKey.getAlgorithm();
            if (!keyAlgorithm.equalsIgnoreCase(publicKey.getAlgorithm())) {
                throw new InvalidKeyException(
                        "Key algorithm of private key #" + (i + 1) + " does not match key"
                                + " algorithm of public key #" + (i + 1) + ": " + keyAlgorithm
                                + " vs " + publicKey.getAlgorithm());
            }
            ApkSignerV2.SignerConfig signerConfig = new ApkSignerV2.SignerConfig();
            signerConfig.privateKey = privateKey;
            signerConfig.certificates = Collections.singletonList(certificate);
            List<Integer> signatureAlgorithms = new ArrayList<>(digestAlgorithms.length);
            for (String digestAlgorithm : digestAlgorithms) {
                try {
                    signatureAlgorithms.add(getV2SignatureAlgorithm(keyAlgorithm, digestAlgorithm));
                } catch (IllegalArgumentException e) {
                    throw new InvalidKeyException(
                            "Unsupported key and digest algorithm combination for signer #"
                                    + (i + 1), e);
                }
            }
            signerConfig.signatureAlgorithms = signatureAlgorithms;
            result.add(signerConfig);
        }
        return result;
    }

    private static int getV2SignatureAlgorithm(String keyAlgorithm, String digestAlgorithm) {
        if ("SHA-256".equalsIgnoreCase(digestAlgorithm)) {
            if ("RSA".equalsIgnoreCase(keyAlgorithm)) {
                // Use RSASSA-PKCS1-v1_5 signature scheme instead of RSASSA-PSS to guarantee
                // deterministic signatures which make life easier for OTA updates (fewer files
                // changed when deterministic signature schemes are used).
                return ApkSignerV2.SIGNATURE_RSA_PKCS1_V1_5_WITH_SHA256;
            } else if ("EC".equalsIgnoreCase(keyAlgorithm)) {
                return ApkSignerV2.SIGNATURE_ECDSA_WITH_SHA256;
            } else if ("DSA".equalsIgnoreCase(keyAlgorithm)) {
                return ApkSignerV2.SIGNATURE_DSA_WITH_SHA256;
            } else {
                throw new IllegalArgumentException("Unsupported key algorithm: " + keyAlgorithm);
            }
        } else if ("SHA-512".equalsIgnoreCase(digestAlgorithm)) {
            if ("RSA".equalsIgnoreCase(keyAlgorithm)) {
                // Use RSASSA-PKCS1-v1_5 signature scheme instead of RSASSA-PSS to guarantee
                // deterministic signatures which make life easier for OTA updates (fewer files
                // changed when deterministic signature schemes are used).
                return ApkSignerV2.SIGNATURE_RSA_PKCS1_V1_5_WITH_SHA512;
            } else if ("EC".equalsIgnoreCase(keyAlgorithm)) {
                return ApkSignerV2.SIGNATURE_ECDSA_WITH_SHA512;
            } else if ("DSA".equalsIgnoreCase(keyAlgorithm)) {
                return ApkSignerV2.SIGNATURE_DSA_WITH_SHA512;
            } else {
                throw new IllegalArgumentException("Unsupported key algorithm: " + keyAlgorithm);
            }
        } else {
            throw new IllegalArgumentException("Unsupported digest algorithm: " + digestAlgorithm);
        }
    }

    public static void sign(X509Certificate cert, PrivateKey key,
                            JarMap inputJar, FileOutputStream outputFile) throws Exception {
        int alignment = 4;
        int hashes = 0;

        X509Certificate[] publicKey = new X509Certificate[1];
        publicKey[0] = cert;
        hashes |= getDigestAlgorithm(publicKey[0]);

        // Set all ZIP file timestamps to Jan 1 2009 00:00:00.
        long timestamp = 1230768000000L;
        // The Java ZipEntry API we're using converts milliseconds since epoch into MS-DOS
        // timestamp using the current timezone. We thus adjust the milliseconds since epoch
        // value to end up with MS-DOS timestamp of Jan 1 2009 00:00:00.
        timestamp -= TimeZone.getDefault().getOffset(timestamp);

        PrivateKey[] privateKey = new PrivateKey[1];
        privateKey[0] = key;

        // Generate, in memory, an APK signed using standard JAR Signature Scheme.
        ByteArrayOutputStream v1SignedApkBuf = new ByteArrayOutputStream();
        JarOutputStream outputJar = new JarOutputStream(v1SignedApkBuf);
        // Use maximum compression for compressed entries because the APK lives forever on
        // the system partition.
        outputJar.setLevel(9);
        Manifest manifest = addDigestsToManifest(inputJar, hashes);
        copyFiles(manifest, inputJar, outputJar, timestamp, alignment);
        signFile(manifest, publicKey, privateKey, timestamp, outputJar);
        outputJar.close();
        ByteBuffer v1SignedApk = ByteBuffer.wrap(v1SignedApkBuf.toByteArray());
        v1SignedApkBuf.reset();

        ByteBuffer[] outputChunks;
        List<ApkSignerV2.SignerConfig> signerConfigs = createV2SignerConfigs(privateKey, publicKey,
                new String[]{APK_SIG_SCHEME_V2_DIGEST_ALGORITHM});
        outputChunks = ApkSignerV2.sign(v1SignedApk, signerConfigs);

        // This assumes outputChunks are array-backed. To avoid this assumption, the
        // code could be rewritten to use FileChannel.
        for (ByteBuffer outputChunk : outputChunks) {
            outputFile.write(outputChunk.array(),
                    outputChunk.arrayOffset() + outputChunk.position(), outputChunk.remaining());
            outputChunk.position(outputChunk.limit());
        }
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
}
