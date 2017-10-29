package com.topjohnwu.magisk.utils;

import android.content.res.AssetManager;

import com.topjohnwu.crypto.SignBoot;

import java.io.FileInputStream;
import java.io.InputStream;

public class BootSigner {

    public static void main(String[] args) throws Exception {
        if ("-verify".equals(args[0])) {
            String certPath = "";
            if (args.length >= 4 && "-certificate".equals(args[2])) {
                /* args[3] is the path to a public key certificate */
                certPath = args[3];
            }
            /* args[1] is the path to a signed boot image */
            boolean signed = SignBoot.verifySignature(args[1],
                    certPath.isEmpty() ? null : new FileInputStream(certPath));
            System.exit(signed ? 0 : 1);
        } else {
            /* args[0] is the target name, typically /boot
               args[1] is the path to a boot image to sign
               args[2] is the path where to output the signed boot image
               args[3] is the path to a private key
               args[4] is the path to the matching public key certificate
            */
            InputStream keyIn, sigIn;
            if (args.length >= 5) {
                keyIn = new FileInputStream(args[3]);
                sigIn = new FileInputStream(args[4]);
            } else {
                /* Use internal test keys */
                AssetManager asset = Utils.getAssets(System.getProperty("java.class.path"));
                if (asset == null)
                    System.exit(1);
                keyIn = asset.open(ZipUtils.PRIVATE_KEY_NAME);
                sigIn = asset.open(ZipUtils.PUBLIC_KEY_NAME);
            }

            SignBoot.doSignature(args[0], args[1], args[2], keyIn, sigIn);
        }
    }
}
