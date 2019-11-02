package com.topjohnwu.signing;

import org.bouncycastle.asn1.ASN1InputStream;
import org.bouncycastle.asn1.ASN1ObjectIdentifier;
import org.bouncycastle.asn1.pkcs.PKCSObjectIdentifiers;
import org.bouncycastle.asn1.pkcs.PrivateKeyInfo;
import org.bouncycastle.asn1.x509.AlgorithmIdentifier;
import org.bouncycastle.asn1.x9.X9ObjectIdentifiers;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.GeneralSecurityException;
import java.security.Key;
import java.security.KeyFactory;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.spec.ECPrivateKeySpec;
import java.security.spec.ECPublicKeySpec;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.HashMap;
import java.util.Map;

public class CryptoUtils {

    static final Map<String, String> ID_TO_ALG;
    static final Map<String, String> ALG_TO_ID;

    static {
        ID_TO_ALG = new HashMap<>();
        ALG_TO_ID = new HashMap<>();
        ID_TO_ALG.put(X9ObjectIdentifiers.ecdsa_with_SHA256.getId(), "SHA256withECDSA");
        ID_TO_ALG.put(X9ObjectIdentifiers.ecdsa_with_SHA384.getId(), "SHA384withECDSA");
        ID_TO_ALG.put(X9ObjectIdentifiers.ecdsa_with_SHA512.getId(), "SHA512withECDSA");
        ID_TO_ALG.put(PKCSObjectIdentifiers.sha1WithRSAEncryption.getId(), "SHA1withRSA");
        ID_TO_ALG.put(PKCSObjectIdentifiers.sha256WithRSAEncryption.getId(), "SHA256withRSA");
        ID_TO_ALG.put(PKCSObjectIdentifiers.sha512WithRSAEncryption.getId(), "SHA512withRSA");
        ALG_TO_ID.put("SHA256withECDSA", X9ObjectIdentifiers.ecdsa_with_SHA256.getId());
        ALG_TO_ID.put("SHA384withECDSA", X9ObjectIdentifiers.ecdsa_with_SHA384.getId());
        ALG_TO_ID.put("SHA512withECDSA", X9ObjectIdentifiers.ecdsa_with_SHA512.getId());
        ALG_TO_ID.put("SHA1withRSA", PKCSObjectIdentifiers.sha1WithRSAEncryption.getId());
        ALG_TO_ID.put("SHA256withRSA", PKCSObjectIdentifiers.sha256WithRSAEncryption.getId());
        ALG_TO_ID.put("SHA512withRSA", PKCSObjectIdentifiers.sha512WithRSAEncryption.getId());
    }

    static String getSignatureAlgorithm(Key key) throws Exception {
        if ("EC".equals(key.getAlgorithm())) {
            int curveSize;
            KeyFactory factory = KeyFactory.getInstance("EC");
            if (key instanceof PublicKey) {
                ECPublicKeySpec spec = factory.getKeySpec(key, ECPublicKeySpec.class);
                curveSize = spec.getParams().getCurve().getField().getFieldSize();
            } else if (key instanceof PrivateKey) {
                ECPrivateKeySpec spec = factory.getKeySpec(key, ECPrivateKeySpec.class);
                curveSize = spec.getParams().getCurve().getField().getFieldSize();
            } else {
                throw new InvalidKeySpecException();
            }
            if (curveSize <= 256) {
                return "SHA256withECDSA";
            } else if (curveSize <= 384) {
                return "SHA384withECDSA";
            } else {
                return "SHA512withECDSA";
            }
        } else if ("RSA".equals(key.getAlgorithm())) {
            return "SHA256withRSA";
        } else {
            throw new IllegalArgumentException("Unsupported key type " + key.getAlgorithm());
        }
    }

    static AlgorithmIdentifier getSignatureAlgorithmIdentifier(Key key) throws Exception {
        String id = ALG_TO_ID.get(getSignatureAlgorithm(key));
        if (id == null) {
            throw new IllegalArgumentException("Unsupported key type " + key.getAlgorithm());
        }
        return new AlgorithmIdentifier(new ASN1ObjectIdentifier(id));
    }

    public static X509Certificate readCertificate(InputStream input)
            throws IOException, GeneralSecurityException {
        try {
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            return (X509Certificate) cf.generateCertificate(input);
        } finally {
            input.close();
        }
    }

    /** Read a PKCS#8 format private key. */
    public static PrivateKey readPrivateKey(InputStream input)
            throws IOException, GeneralSecurityException {
        try {
            ByteArrayStream buf = new ByteArrayStream();
            buf.readFrom(input);
            byte[] bytes = buf.toByteArray();
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
}
