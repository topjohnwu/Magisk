package com.topjohnwu.magisk.utils;

import android.support.annotation.Keep;

import com.topjohnwu.crypto.SignBoot;

import java.io.FileInputStream;
import java.io.InputStream;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

public class BootSigner {

    @Keep
    public static void main(String[] args) throws Exception {
        if (args.length > 0 && "-verify".equals(args[0])) {
            String certPath = "";
            if (args.length >= 3 && "-certificate".equals(args[1])) {
                /* args[2] is the path to a public key certificate */
                certPath = args[2];
            }
            /* args[1] is the path to a signed boot image */
            boolean signed = SignBoot.verifySignature(System.in,
                    certPath.isEmpty() ? null : new FileInputStream(certPath));
            System.exit(signed ? 0 : 1);
        } else if (args.length > 0 && "-sign".equals(args[0])) {
            InputStream keyIn, certIn;
            if (args.length >= 3) {
                keyIn = new FileInputStream(args[1]);
                certIn = new FileInputStream(args[2]);
            } else {
                /* Use internal test keys */
                JarFile apk = new JarFile(System.getProperty("java.class.path"));
                JarEntry keyEntry = apk.getJarEntry("assets/" + Const.PRIVATE_KEY_NAME);
                JarEntry sigEntry = apk.getJarEntry("assets/" + Const.PUBLIC_KEY_NAME);

                keyIn = apk.getInputStream(keyEntry);
                certIn = apk.getInputStream(sigEntry);
            }

            boolean success = SignBoot.doSignature("/boot", System.in, System.out, keyIn, certIn);
            System.exit(success ? 0 : 1);
        } else {
            System.err.println(
                    "BootSigner <actions> [args]\n" +
                    "Input from stdin, outputs to stdout\n" +
                    "\n" +
                    "Actions:\n" +
                    "   -verify [x509.pem]\n" +
                    "      verify image, cert is optional\n" +
                    "   -sign [pk8] [x509.pem]\n" +
                    "      sign image, key and cert are optional\n"
            );
        }
    }
}
