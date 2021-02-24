package com.topjohnwu.signing;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.bouncycastle.asn1.ASN1Encodable;
import org.bouncycastle.asn1.ASN1EncodableVector;
import org.bouncycastle.asn1.ASN1InputStream;
import org.bouncycastle.asn1.ASN1Integer;
import org.bouncycastle.asn1.ASN1Object;
import org.bouncycastle.asn1.ASN1ObjectIdentifier;
import org.bouncycastle.asn1.ASN1Primitive;
import org.bouncycastle.asn1.ASN1Sequence;
import org.bouncycastle.asn1.DEROctetString;
import org.bouncycastle.asn1.DERPrintableString;
import org.bouncycastle.asn1.DERSequence;
import org.bouncycastle.asn1.x509.AlgorithmIdentifier;

import java.io.ByteArrayInputStream;
import java.io.FileInputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.Signature;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.Arrays;

@Keep
public class SignBoot {

    private static final int BOOT_IMAGE_HEADER_V1_RECOVERY_DTBO_SIZE_OFFSET = 1632;
    private static final int BOOT_IMAGE_HEADER_V2_DTB_SIZE_OFFSET = 1648;

    // Arbitrary maximum header version value; when greater assume the field is dt/extra size
    private static final int BOOT_IMAGE_HEADER_VERSION_MAXIMUM = 8;

    // Maximum header size byte value to read (currently the bootimg minimum page size)
    private static final int BOOT_IMAGE_HEADER_SIZE_MAXIMUM = 2048;

    private static class PushBackRWStream extends FilterInputStream {
        private OutputStream out;
        private int pos = 0;
        private byte[] backBuf;

        PushBackRWStream(InputStream in, OutputStream o) {
            super(in);
            out = o;
        }

        @Override
        public int read() throws IOException {
            int b;
            if (backBuf != null && backBuf.length > pos) {
                b = backBuf[pos++];
            } else {
                b = super.read();
                out.write(b);
            }
            return b;
        }

        @Override
        public int read(byte[] bytes, int off, int len) throws IOException {
            int read = 0;
            if (backBuf != null && backBuf.length > pos) {
                read = Math.min(len, backBuf.length - pos);
                System.arraycopy(backBuf, pos, bytes, off, read);
                pos += read;
                off += read;
                len -= read;
            }
            if (len > 0) {
                int ar = super.read(bytes, off, len);
                read += ar;
                out.write(bytes, off, ar);
            }
            return read;
        }

        void unread(byte[] buf) {
            backBuf = buf;
        }
    }

    private static int fullRead(InputStream in, byte[] b) throws IOException {
        return fullRead(in, b, 0, b.length);
    }

    private static int fullRead(InputStream in, byte[] b, int off, int len) throws IOException {
        int n = 0;
        while (n < len) {
            int count = in.read(b, off + n, len - n);
            if (count <= 0)
                break;
            n += count;
        }
        return n;
    }

    public static boolean doSignature(
            @Nullable X509Certificate cert, @Nullable PrivateKey key,
            @NonNull InputStream imgIn, @NonNull OutputStream imgOut, @NonNull String target
    ) {
        try {
            PushBackRWStream in = new PushBackRWStream(imgIn, imgOut);
            byte[] hdr = new byte[BOOT_IMAGE_HEADER_SIZE_MAXIMUM];
            // First read the header
            fullRead(in, hdr);
            int signableSize = getSignableImageSize(hdr);
            // Unread header
            in.unread(hdr);
            BootSignature bootsig = new BootSignature(target, signableSize);
            if (cert == null) {
                cert = CryptoUtils.readCertificate(
                        new ByteArrayInputStream(KeyData.verityCert()));
            }
            bootsig.setCertificate(cert);
            if (key == null) {
                key = CryptoUtils.readPrivateKey(
                        new ByteArrayInputStream(KeyData.verityKey()));
            }
            byte[] sig = bootsig.sign(key, in, signableSize);
            bootsig.setSignature(sig, CryptoUtils.getSignatureAlgorithmIdentifier(key));
            byte[] encoded_bootsig = bootsig.getEncoded();
            imgOut.write(encoded_bootsig);
            imgOut.flush();
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public static boolean verifySignature(InputStream imgIn, X509Certificate cert) {
        try {
            // Read the header for size
            byte[] hdr = new byte[BOOT_IMAGE_HEADER_SIZE_MAXIMUM];
            if (fullRead(imgIn, hdr) != hdr.length) {
                System.err.println("Unable to read image header");
                return false;
            }
            int signableSize = getSignableImageSize(hdr);

            // Read the rest of the image
            byte[] rawImg = Arrays.copyOf(hdr, signableSize);
            int remain = signableSize - hdr.length;
            if (fullRead(imgIn, rawImg, hdr.length, remain) != remain) {
                System.err.println("Unable to read image");
                return false;
            }

            // Read footer, which contains the signature
            byte[] signature = new byte[4096];
            if (imgIn.read(signature) == -1 || Arrays.equals(signature, new byte [signature.length])) {
                System.err.println("Invalid image: not signed");
                return false;
            }

            BootSignature bootsig = new BootSignature(signature);
            if (cert != null) {
                bootsig.setCertificate(cert);
            }
            if (bootsig.verify(rawImg, signableSize)) {
                System.err.println("Signature is VALID");
                return true;
            } else {
                System.err.println("Signature is INVALID");
            }
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
        return false;
    }

    public static int getSignableImageSize(byte[] data) throws Exception {
        if (!Arrays.equals(Arrays.copyOfRange(data, 0, 8),
                "ANDROID!".getBytes("US-ASCII"))) {
            throw new IllegalArgumentException("Invalid image header: missing magic");
        }
        ByteBuffer image = ByteBuffer.wrap(data);
        image.order(ByteOrder.LITTLE_ENDIAN);
        image.getLong(); // magic
        int kernelSize = image.getInt();
        image.getInt(); // kernel_addr
        int ramdskSize = image.getInt();
        image.getInt(); // ramdisk_addr
        int secondSize = image.getInt();
        image.getLong(); // second_addr + tags_addr
        int pageSize = image.getInt();
        if (pageSize >= 0x02000000) {
            throw new IllegalArgumentException("Invalid image header: PXA header detected");
        }
        int length = pageSize // include the page aligned image header
                + ((kernelSize + pageSize - 1) / pageSize) * pageSize
                + ((ramdskSize + pageSize - 1) / pageSize) * pageSize
                + ((secondSize + pageSize - 1) / pageSize) * pageSize;
        int headerVersion = image.getInt(); // boot image header version or dt/extra size
        if (headerVersion > 0 && headerVersion < BOOT_IMAGE_HEADER_VERSION_MAXIMUM) {
            image.position(BOOT_IMAGE_HEADER_V1_RECOVERY_DTBO_SIZE_OFFSET);
            int recoveryDtboLength = image.getInt();
            length += ((recoveryDtboLength + pageSize - 1) / pageSize) * pageSize;
            image.getLong(); // recovery_dtbo address
            int headerSize = image.getInt();
            if (headerVersion == 2) {
                image.position(BOOT_IMAGE_HEADER_V2_DTB_SIZE_OFFSET);
                int dtbLength = image.getInt();
                length += ((dtbLength + pageSize - 1) / pageSize) * pageSize;
                image.getLong(); // dtb address
            }
            if (image.position() != headerSize) {
                throw new IllegalArgumentException("Invalid image header: invalid header length");
            }
        } else {
            // headerVersion is 0 or actually dt/extra size in this case
            length += ((headerVersion + pageSize - 1) / pageSize) * pageSize;
        }
        length = ((length + pageSize - 1) / pageSize) * pageSize;
        if (length <= 0) {
            throw new IllegalArgumentException("Invalid image header: invalid length");
        }
        return length;
    }

    static class BootSignature extends ASN1Object {
        private ASN1Integer formatVersion;
        private ASN1Encodable certificate;
        private AlgorithmIdentifier algId;
        private DERPrintableString target;
        private ASN1Integer length;
        private DEROctetString signature;
        private PublicKey publicKey;
        private static final int FORMAT_VERSION = 1;

        /**
         * Initializes the object for signing an image file
         * @param target Target name, included in the signed data
         * @param length Length of the image, included in the signed data
         */
        public BootSignature(String target, int length) {
            this.formatVersion = new ASN1Integer(FORMAT_VERSION);
            this.target = new DERPrintableString(target);
            this.length = new ASN1Integer(length);
        }

        /**
         * Initializes the object for verifying a signed image file
         * @param signature Signature footer
         */
        public BootSignature(byte[] signature) throws Exception {
            ASN1InputStream stream = new ASN1InputStream(signature);
            ASN1Sequence sequence = (ASN1Sequence) stream.readObject();
            formatVersion = (ASN1Integer) sequence.getObjectAt(0);
            if (formatVersion.getValue().intValue() != FORMAT_VERSION) {
                throw new IllegalArgumentException("Unsupported format version");
            }
            certificate = sequence.getObjectAt(1);
            byte[] encoded = ((ASN1Object) certificate).getEncoded();
            ByteArrayInputStream bis = new ByteArrayInputStream(encoded);
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            X509Certificate c = (X509Certificate) cf.generateCertificate(bis);
            publicKey = c.getPublicKey();
            ASN1Sequence algId = (ASN1Sequence) sequence.getObjectAt(2);
            this.algId = new AlgorithmIdentifier((ASN1ObjectIdentifier) algId.getObjectAt(0));
            ASN1Sequence attrs = (ASN1Sequence) sequence.getObjectAt(3);
            target = (DERPrintableString) attrs.getObjectAt(0);
            length = (ASN1Integer) attrs.getObjectAt(1);
            this.signature = (DEROctetString) sequence.getObjectAt(4);
        }

        public ASN1Object getAuthenticatedAttributes() {
            ASN1EncodableVector attrs = new ASN1EncodableVector();
            attrs.add(target);
            attrs.add(length);
            return new DERSequence(attrs);
        }

        public byte[] getEncodedAuthenticatedAttributes() throws IOException {
            return getAuthenticatedAttributes().getEncoded();
        }

        public void setSignature(byte[] sig, AlgorithmIdentifier algId) {
            this.algId = algId;
            signature = new DEROctetString(sig);
        }

        public void setCertificate(X509Certificate cert)
                throws CertificateEncodingException, IOException {
            ASN1InputStream s = new ASN1InputStream(cert.getEncoded());
            certificate = s.readObject();
            publicKey = cert.getPublicKey();
        }

        public byte[] sign(PrivateKey key, InputStream is, int len) throws Exception {
            Signature signer = Signature.getInstance(CryptoUtils.getSignatureAlgorithm(key));
            signer.initSign(key);
            int read;
            byte buffer[] = new byte[4096];
            while ((read = is.read(buffer, 0, Math.min(len, buffer.length))) > 0) {
                signer.update(buffer, 0, read);
                len -= read;
            }
            signer.update(getEncodedAuthenticatedAttributes());
            return signer.sign();
        }

        public boolean verify(byte[] image, int length) throws Exception {
            if (this.length.getValue().intValue() != length) {
                throw new IllegalArgumentException("Invalid image length");
            }
            String algName = CryptoUtils.ID_TO_ALG.get(algId.getAlgorithm().getId());
            if (algName == null) {
                throw new IllegalArgumentException("Unsupported algorithm " + algId.getAlgorithm());
            }
            Signature verifier = Signature.getInstance(algName);
            verifier.initVerify(publicKey);
            verifier.update(image, 0, length);
            verifier.update(getEncodedAuthenticatedAttributes());
            return verifier.verify(signature.getOctets());
        }

        @Override
        public ASN1Primitive toASN1Primitive() {
            ASN1EncodableVector v = new ASN1EncodableVector();
            v.add(formatVersion);
            v.add(certificate);
            v.add(algId);
            v.add(getAuthenticatedAttributes());
            v.add(signature);
            return new DERSequence(v);
        }

    }

    public static void main(String[] args) throws Exception {
        if (args.length > 0 && "-verify".equals(args[0])) {
            X509Certificate cert = null;
            if (args.length >= 2) {
                // args[1] is the path to a public key certificate
                cert = CryptoUtils.readCertificate(new FileInputStream(args[1]));
            }
            boolean signed = SignBoot.verifySignature(System.in, cert);
            System.exit(signed ? 0 : 1);
        } else if (args.length > 0 && "-sign".equals(args[0])) {
            X509Certificate cert = null;
            PrivateKey key = null;
            String name = "/boot";

            if (args.length >= 3) {
                cert = CryptoUtils.readCertificate(new FileInputStream(args[1]));
                key = CryptoUtils.readPrivateKey(new FileInputStream(args[2]));
            }
            if (args.length == 2) {
                name = args[1];
            } else if (args.length >= 4) {
                name = args[3];
            }

            boolean result = SignBoot.doSignature(cert, key, System.in, System.out, name);
            System.exit(result ? 0 : 1);
        } else {
            System.err.println(
                    "BootSigner <actions> [args]\n" +
                    "Input from stdin, output to stdout\n" +
                    "\n" +
                    "Actions:\n" +
                    "   -verify [x509.pem]\n" +
                    "      verify image. cert is optional.\n" +
                    "   -sign [x509.pem] [pk8] [name]\n" +
                    "      sign image. name and the cert/key pair are optional.\n" +
                    "      name should be either /boot (default) or /recovery.\n"
            );
        }
    }

}
