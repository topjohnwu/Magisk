package com.topjohnwu.crypto;

import org.bouncycastle.jce.provider.BouncyCastleProvider;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.security.Security;

public class ZipSigner {
    public static void main(String[] args) {
        boolean minSign = false;
        int argStart = 0;

        if (args.length < 4) {
            System.err.println("Usage: zipsigner [-m] publickey.x509[.pem] privatekey.pk8 input.jar output.jar");
            System.exit(2);
        }

        if (args[0].equals("-m")) {
            minSign = true;
            argStart = 1;
        }

        SignAPK.sBouncyCastleProvider = new BouncyCastleProvider();
        Security.insertProviderAt(SignAPK.sBouncyCastleProvider, 1);

        File pubKey = new File(args[argStart]);
        File privKey = new File(args[argStart + 1]);
        File input = new File(args[argStart + 2]);
        File output = new File(args[argStart + 3]);

        try (InputStream pub = new FileInputStream(pubKey);
             InputStream priv = new FileInputStream(privKey);
             JarMap jar = new JarMap(input, false)) {
            SignAPK.signZip(pub, priv, jar, output, minSign);
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }
}
