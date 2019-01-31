package com.topjohnwu.signing;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

public class ZipSigner {

    public static void usage() {
        System.err.println("ZipSigner usage:");
        System.err.println("  zipsigner.jar input.jar output.jar");
        System.err.println("    sign jar with AOSP test keys");
        System.err.println("  zipsigner.jar x509.pem pk8 input.jar output.jar");
        System.err.println("    sign jar with certificate / private key pair");
        System.err.println("  zipsigner.jar jks keyStorePass keyAlias keyPass input.jar output.jar");
        System.err.println("    sign jar with Java KeyStore");
        System.exit(2);
    }

    public static void main(String[] args) throws Exception {
        if (args.length != 2 && args.length != 4 && args.length != 6)
            usage();

        try (JarMap in = new JarMap(args[args.length - 2], false);
             OutputStream out = new FileOutputStream(args[args.length - 1])) {
            if (args.length == 2) {
                SignAPK.sign(in, out);
            } else if (args.length == 4) {
                try (InputStream cert = new FileInputStream(args[0]);
                     InputStream key = new FileInputStream(args[1])) {
                    SignAPK.sign(cert, key, in, out);
                }
            } else if (args.length == 6) {
                try (InputStream jks = new FileInputStream(args[0])) {
                    SignAPK.sign(jks, args[1], args[2], args[3], in, out);
                }
            }
        }
    }
}
