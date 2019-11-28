package com.topjohnwu.signing;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.Security;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

public class ZipSigner {

    private static void usage() {
        System.err.println("ZipSigner usage:");
        System.err.println("  zipsigner.jar input.jar output.jar");
        System.err.println("    sign jar with AOSP test keys");
        System.err.println("  zipsigner.jar x509.pem pk8 input.jar output.jar");
        System.err.println("    sign jar with certificate / private key pair");
        System.err.println("  zipsigner.jar keyStore keyStorePass alias keyPass input.jar output.jar");
        System.err.println("    sign jar with Java KeyStore");
        System.exit(2);
    }

    private static void sign(JarMap input, OutputStream output) throws Exception {
        sign(SignAPK.class.getResourceAsStream("/keys/testkey.x509.pem"),
                SignAPK.class.getResourceAsStream("/keys/testkey.pk8"), input, output);
    }

    private static void sign(InputStream certIs, InputStream keyIs,
                             JarMap input, OutputStream output) throws Exception {
        X509Certificate cert = CryptoUtils.readCertificate(certIs);
        PrivateKey key = CryptoUtils.readPrivateKey(keyIs);
        SignAPK.signAndAdjust(cert, key, input, output);
    }

    private static void sign(String keyStore, String keyStorePass, String alias, String keyPass,
                             JarMap in, OutputStream out) throws Exception {
        KeyStore ks;
        try {
            ks = KeyStore.getInstance("JKS");
            try (InputStream is = new FileInputStream(keyStore)) {
                ks.load(is, keyStorePass.toCharArray());
            }
        } catch (KeyStoreException|IOException|CertificateException|NoSuchAlgorithmException e) {
            ks = KeyStore.getInstance("PKCS12");
            try (InputStream is = new FileInputStream(keyStore)) {
                ks.load(is, keyStorePass.toCharArray());
            }
        }
        X509Certificate cert = (X509Certificate) ks.getCertificate(alias);
        PrivateKey key = (PrivateKey) ks.getKey(alias, keyPass.toCharArray());
        SignAPK.signAndAdjust(cert, key, in, out);
    }

    public static void main(String[] args) throws Exception {
        if (args.length != 2 && args.length != 4 && args.length != 6)
            usage();

        Security.insertProviderAt(new BouncyCastleProvider(), 1);

        try (JarMap in = JarMap.open(args[args.length - 2], false);
             OutputStream out = new FileOutputStream(args[args.length - 1])) {
            if (args.length == 2) {
                sign(in, out);
            } else if (args.length == 4) {
                try (InputStream cert = new FileInputStream(args[0]);
                     InputStream key = new FileInputStream(args[1])) {
                    sign(cert, key, in, out);
                }
            } else if (args.length == 6) {
                sign(args[0], args[1], args[2], args[3], in, out);
            }
        }
    }
}
