package com.topjohnwu.crypto;

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
import org.bouncycastle.jce.provider.BouncyCastleProvider;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.Security;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.Arrays;

public class SignBoot {

    static {
        Security.addProvider(new BouncyCastleProvider());
    }

    public static boolean doSignature(String target, InputStream imgIn, OutputStream imgOut,
                                   InputStream keyIn, InputStream certIn) {
        try {
            ByteArrayStream bas = new ByteArrayStream();
            bas.readFrom(imgIn);
            byte[] image = bas.toByteArray();
            bas.close();
            int signableSize = getSignableImageSize(image);
            if (signableSize < image.length) {
                System.err.println("NOTE: truncating input from " +
                        image.length + " to " + signableSize + " bytes");
                image = Arrays.copyOf(image, signableSize);
            } else if (signableSize > image.length) {
                throw new IllegalArgumentException("Invalid image: too short, expected " +
                        signableSize + " bytes");
            }
            BootSignature bootsig = new BootSignature(target, image.length);
            X509Certificate cert = CryptoUtils.readPublicKey(certIn);
            bootsig.setCertificate(cert);
            PrivateKey key = CryptoUtils.readPrivateKey(keyIn);
            bootsig.setSignature(bootsig.sign(image, key),
                    CryptoUtils.getSignatureAlgorithmIdentifier(key));
            byte[] encoded_bootsig = bootsig.getEncoded();
            imgOut.write(image);
            imgOut.write(encoded_bootsig);
            imgOut.flush();
            return true;
        } catch (Exception e) {
            e.printStackTrace(System.err);
            return false;
        }
    }

    public static boolean verifySignature(InputStream imgIn, InputStream certPath) {
        try {
            ByteArrayStream bas = new ByteArrayStream();
            bas.readFrom(imgIn);
            byte[] image = bas.toByteArray();
            bas.close();
            int signableSize = getSignableImageSize(image);
            if (signableSize >= image.length) {
                System.err.println("Invalid image: not signed");
                return false;
            }
            byte[] signature = Arrays.copyOfRange(image, signableSize, image.length);
            BootSignature bootsig = new BootSignature(signature);
            if (certPath != null) {
                bootsig.setCertificate(CryptoUtils.readPublicKey(certPath));
            }
            if (bootsig.verify(Arrays.copyOf(image, signableSize))) {
                System.err.println("Signature is VALID");
                return true;
            } else {
                System.err.println("Signature is INVALID");
            }
        } catch (Exception e) {
            e.printStackTrace(System.err);
            System.err.println("Invalid image: not signed");
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
        int length = pageSize // include the page aligned image header
                + ((kernelSize + pageSize - 1) / pageSize) * pageSize
                + ((ramdskSize + pageSize - 1) / pageSize) * pageSize
                + ((secondSize + pageSize - 1) / pageSize) * pageSize;
        length = ((length + pageSize - 1) / pageSize) * pageSize;
        if (length <= 0) {
            throw new IllegalArgumentException("Invalid image header: invalid length");
        }
        return length;
    }

    static class BootSignature extends ASN1Object {
        private ASN1Integer formatVersion;
        private ASN1Encodable certificate;
        private AlgorithmIdentifier algorithmIdentifier;
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
        public BootSignature(byte[] signature)
                throws Exception {
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
            algorithmIdentifier = new AlgorithmIdentifier(
                    (ASN1ObjectIdentifier) algId.getObjectAt(0));
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
            algorithmIdentifier = algId;
            signature = new DEROctetString(sig);
        }

        public void setCertificate(X509Certificate cert)
                throws Exception, IOException, CertificateEncodingException {
            ASN1InputStream s = new ASN1InputStream(cert.getEncoded());
            certificate = s.readObject();
            publicKey = cert.getPublicKey();
        }

        public byte[] generateSignableImage(byte[] image) throws IOException {
            byte[] attrs = getEncodedAuthenticatedAttributes();
            byte[] signable = Arrays.copyOf(image, image.length + attrs.length);
            for (int i=0; i < attrs.length; i++) {
                signable[i+image.length] = attrs[i];
            }
            return signable;
        }

        public byte[] sign(byte[] image, PrivateKey key) throws Exception {
            byte[] signable = generateSignableImage(image);
            return CryptoUtils.sign(key, signable);
        }

        public boolean verify(byte[] image) throws Exception {
            if (length.getValue().intValue() != image.length) {
                throw new IllegalArgumentException("Invalid image length");
            }
            byte[] signable = generateSignableImage(image);
            return CryptoUtils.verify(publicKey, signable, signature.getOctets(),
                    algorithmIdentifier);
        }

        @Override
        public ASN1Primitive toASN1Primitive() {
            ASN1EncodableVector v = new ASN1EncodableVector();
            v.add(formatVersion);
            v.add(certificate);
            v.add(algorithmIdentifier);
            v.add(getAuthenticatedAttributes());
            v.add(signature);
            return new DERSequence(v);
        }

    }
}
