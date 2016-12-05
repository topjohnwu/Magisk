/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* This is just a copy/paste/cut job from original SignAPK sources. This 
 * adaptation adds only the whole-file signature to a ZIP(jar,apk) file, and 
 * doesn't do any of the per-file signing, creating manifests, etc. This is 
 * useful when you've changed the structure itself of an existing (signed!) 
 * ZIP file, but the extracted contents are still identical. Using
 * the normal SignAPK may re-arrange other things inside the ZIP, which may
 * be unwanted behavior. This version only changes the ZIP's tail and keeps
 * the rest the same - CF
 */

package eu.chainfire.minsignapk;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.GeneralSecurityException;
import java.security.KeyFactory;
import java.security.PrivateKey;
import java.security.Signature;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.security.spec.PKCS8EncodedKeySpec;

import sun.security.pkcs.ContentInfo;
import sun.security.pkcs.PKCS7;
import sun.security.pkcs.SignerInfo;
import sun.security.x509.AlgorithmId;
import sun.security.x509.X500Name;

public class MinSignAPK {
	/** Write a .RSA file with a digital signature. */
    private static void writeSignatureBlock(Signature signature, X509Certificate publicKey, OutputStream out)
            throws IOException, GeneralSecurityException {
        SignerInfo signerInfo = new SignerInfo(new X500Name(publicKey.getIssuerX500Principal().getName()),
                publicKey.getSerialNumber(), AlgorithmId.get("SHA1"), AlgorithmId.get("RSA"), signature.sign());

        PKCS7 pkcs7 = new PKCS7(new AlgorithmId[] { AlgorithmId.get("SHA1") }, new ContentInfo(ContentInfo.DATA_OID,
                null), new X509Certificate[] { publicKey }, new SignerInfo[] { signerInfo });

        pkcs7.encodeSignedData(out);
    }
    
	private static void signWholeOutputFile(byte[] zipData, OutputStream outputStream, X509Certificate publicKey,
            PrivateKey privateKey) throws IOException, GeneralSecurityException {

        // For a zip with no archive comment, the
        // end-of-central-directory record will be 22 bytes long, so
        // we expect to find the EOCD marker 22 bytes from the end.
        if (zipData[zipData.length - 22] != 0x50 || zipData[zipData.length - 21] != 0x4b
                || zipData[zipData.length - 20] != 0x05 || zipData[zipData.length - 19] != 0x06) {
            throw new IllegalArgumentException("zip data already has an archive comment");
        }

        Signature signature = Signature.getInstance("SHA1withRSA");
        signature.initSign(privateKey);
        signature.update(zipData, 0, zipData.length - 2);

        ByteArrayOutputStream temp = new ByteArrayOutputStream();

        // put a readable message and a null char at the start of the
        // archive comment, so that tools that display the comment
        // (hopefully) show something sensible.
        // TODO: anything more useful we can put in this message?
        byte[] message = "signed by SignApk".getBytes("UTF-8");
        temp.write(message);
        temp.write(0);
        writeSignatureBlock(signature, publicKey, temp);
        int total_size = temp.size() + 6;
        if (total_size > 0xffff) {
            throw new IllegalArgumentException("signature is too big for ZIP file comment");
        }
        // signature starts this many bytes from the end of the file
        int signature_start = total_size - message.length - 1;
        temp.write(signature_start & 0xff);
        temp.write((signature_start >> 8) & 0xff);
        // Why the 0xff bytes? In a zip file with no archive comment,
        // bytes [-6:-2] of the file are the little-endian offset from
        // the start of the file to the central directory. So for the
        // two high bytes to be 0xff 0xff, the archive would have to
        // be nearly 4GB in side. So it's unlikely that a real
        // commentless archive would have 0xffs here, and lets us tell
        // an old signed archive from a new one.
        temp.write(0xff);
        temp.write(0xff);
        temp.write(total_size & 0xff);
        temp.write((total_size >> 8) & 0xff);
        temp.flush();

        // Signature verification checks that the EOCD header is the
        // last such sequence in the file (to avoid minzip finding a
        // fake EOCD appended after the signature in its scan). The
        // odds of producing this sequence by chance are very low, but
        // let's catch it here if it does.
        byte[] b = temp.toByteArray();
        for (int i = 0; i < b.length - 3; ++i) {
            if (b[i] == 0x50 && b[i + 1] == 0x4b && b[i + 2] == 0x05 && b[i + 3] == 0x06) {
                throw new IllegalArgumentException("found spurious EOCD header at " + i);
            }
        }

        outputStream.write(zipData, 0, zipData.length - 2);
        outputStream.write(total_size & 0xff);
        outputStream.write((total_size >> 8) & 0xff);
        temp.writeTo(outputStream);
    }
	
	private static PrivateKey readPrivateKey(File file)
	        throws IOException, GeneralSecurityException {
		DataInputStream input = new DataInputStream(new FileInputStream(file));
		try {
			byte[] bytes = new byte[(int) file.length()];
			input.read(bytes);

			// dont support encrypted keys atm
			//KeySpec spec = decryptPrivateKey(bytes, file);
			//if (spec == null) {
				KeySpec spec = new PKCS8EncodedKeySpec(bytes);
			//}

			try {
				return KeyFactory.getInstance("RSA").generatePrivate(spec);
			} catch (InvalidKeySpecException ex) {
				return KeyFactory.getInstance("DSA").generatePrivate(spec);
			}
		} finally {
			input.close();
		}
	}
	
	private static X509Certificate readPublicKey(File file)
			throws IOException, GeneralSecurityException {
		FileInputStream input = new FileInputStream(file);
		try {
			CertificateFactory cf = CertificateFactory.getInstance("X.509");
			return (X509Certificate) cf.generateCertificate(input);
		} finally {
			input.close();
		}
	}	
	
	public static void main(String[] args) {
		if (args.length < 4) {
			System.out.println("MinSignAPK pemfile pk8file inzip outzip");
			System.out.println("- only adds whole-file signature to zip");
			return;
		}
		 
		String pemFile = args[0];
		String pk8File = args[1];
		String inFile = args[2];
		String outFile = args[3];
		
		try {
			X509Certificate publicKey = readPublicKey(new File(pemFile));
			PrivateKey privateKey = readPrivateKey(new File(pk8File));
			
			InputStream fis = new FileInputStream(inFile);
			byte[] buffer = new byte[(int)(new File(inFile)).length()];
			fis.read(buffer);
			fis.close();
			
			OutputStream fos = new FileOutputStream(outFile, false);
			signWholeOutputFile(buffer, fos, publicKey, privateKey);
			fos.close();			
		} catch (Exception e) {
			e.printStackTrace();
		}
	 }
}
