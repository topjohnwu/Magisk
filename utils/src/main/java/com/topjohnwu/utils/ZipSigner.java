package com.topjohnwu.utils;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.security.Security;

public class ZipSigner {

    public static void usage() {
        System.err.println("Usage: zipsigner [x509.pem] [pk8] input.jar output.jar");
        System.err.println("Note: If no certificate/private key pair is specified, it will use the embedded test keys.");
        System.exit(2);
    }

    public static void main(String[] args) throws Exception {
        int argStart = 0;

        if (args.length < 2)
            usage();

        InputStream cert = null;
        InputStream key = null;

        if (args.length - argStart == 4) {
            cert = new FileInputStream(new File(args[argStart]));
            key = new FileInputStream(new File(args[argStart + 1]));
            argStart += 2;
        }

        if (args.length - argStart != 2)
            usage();

        SignAPK.sBouncyCastleProvider = new BouncyCastleProvider();
        Security.insertProviderAt(SignAPK.sBouncyCastleProvider, 1);

        File input = new File(args[argStart]);
        File output = new File(args[argStart + 1]);

        try (JarMap jar = new JarMap(input, false);
             BufferedOutputStream out = new BufferedOutputStream(new FileOutputStream(output))) {
            SignAPK.signZip(cert, key, jar, out);
        }
    }
}
