package com.topjohnwu.magisk.utils;

import com.topjohnwu.utils.SignBoot;

import java.io.FileInputStream;
import java.io.InputStream;

import androidx.annotation.Keep;

public class BootSigner {

    public static void main(String[] args) throws Exception {
        if (args.length > 0 && "-verify".equals(args[0])) {
            String certPath = "";
            if (args.length >= 2) {
                /* args[1] is the path to a public key certificate */
                certPath = args[1];
            }
            boolean signed = SignBoot.verifySignature(System.in,
                    certPath.isEmpty() ? null : new FileInputStream(certPath));
            System.exit(signed ? 0 : 1);
        } else if (args.length > 0 && "-sign".equals(args[0])) {
            InputStream cert = null;
            InputStream key = null;

            if (args.length >= 3) {
                cert = new FileInputStream(args[1]);
                key = new FileInputStream(args[2]);
            }

            boolean success = SignBoot.doSignature("/boot", System.in, System.out, cert, key);
            System.exit(success ? 0 : 1);
        } else {
            System.err.println(
                    "BootSigner <actions> [args]\n" +
                    "Input from stdin, outputs to stdout\n" +
                    "\n" +
                    "Actions:\n" +
                    "   -verify [x509.pem]\n" +
                    "      verify image, cert is optional\n" +
                    "   -sign [x509.pem] [pk8]\n" +
                    "      sign image, cert and key pair is optional\n"
            );
        }
    }
}
