use der::referenced::OwnedToRef;
use der::{Decode, DecodePem, Encode, Sequence, SliceReader};
use digest::DynDigest;
use p256::ecdsa::{
    Signature as P256Signature, SigningKey as P256SigningKey, VerifyingKey as P256VerifyingKey,
};
use p256::pkcs8::DecodePrivateKey;
use p384::ecdsa::{
    Signature as P384Signature, SigningKey as P384SigningKey, VerifyingKey as P384VerifyingKey,
};
use p521::ecdsa::{
    Signature as P521Signature, SigningKey as P521SigningKey, VerifyingKey as P521VerifyingKey,
};
use rsa::pkcs1v15::{
    Signature as RsaSignature, SigningKey as RsaSigningKey, VerifyingKey as RsaVerifyingKey,
};
use rsa::pkcs8::SubjectPublicKeyInfoRef;
use rsa::signature::SignatureEncoding;
use rsa::signature::hazmat::{PrehashSigner, PrehashVerifier};
use rsa::{RsaPrivateKey, RsaPublicKey};
use sha1::Sha1;
use sha2::{Sha256, Sha384, Sha512};
use x509_cert::Certificate;
use x509_cert::der::Any;
use x509_cert::der::asn1::{OctetString, PrintableString};
use x509_cert::spki::AlgorithmIdentifier;

use base::libc::c_char;
use base::{LoggedResult, MappedFile, ResultExt, StrErr, Utf8CStr, log_err};

use crate::ffi::BootImage;

#[allow(clippy::upper_case_acronyms)]
pub enum SHA {
    SHA1(Sha1),
    SHA256(Sha256),
}

impl SHA {
    pub fn update(&mut self, data: &[u8]) {
        match self {
            SHA::SHA1(h) => h.update(data),
            SHA::SHA256(h) => h.update(data),
        }
    }

    pub fn output_size(&self) -> usize {
        match self {
            SHA::SHA1(h) => h.output_size(),
            SHA::SHA256(h) => h.output_size(),
        }
    }

    pub fn finalize_into(&mut self, out: &mut [u8]) {
        match self {
            SHA::SHA1(h) => h.finalize_into_reset(out),
            SHA::SHA256(h) => h.finalize_into_reset(out),
        }
        .ok();
    }
}

pub fn get_sha(use_sha1: bool) -> Box<SHA> {
    Box::new(if use_sha1 {
        SHA::SHA1(Sha1::default())
    } else {
        SHA::SHA256(Sha256::default())
    })
}

pub fn sha1_hash(data: &[u8], out: &mut [u8]) {
    let mut h = Sha1::default();
    h.update(data);
    DynDigest::finalize_into(h, out).ok();
}

pub fn sha256_hash(data: &[u8], out: &mut [u8]) {
    let mut h = Sha256::default();
    h.update(data);
    DynDigest::finalize_into(h, out).ok();
}

#[allow(clippy::large_enum_variant)]
enum SigningKey {
    SHA256withRSA(RsaSigningKey<Sha256>),
    SHA256withECDSA(P256SigningKey),
    SHA384withECDSA(P384SigningKey),
    SHA521withECDSA(P521SigningKey),
}

#[allow(clippy::large_enum_variant)]
enum VerifyingKey {
    SHA256withRSA(RsaVerifyingKey<Sha256>),
    SHA256withECDSA(P256VerifyingKey),
    SHA384withECDSA(P384VerifyingKey),
    SHA521withECDSA(P521VerifyingKey),
}

struct Verifier {
    digest: Box<dyn DynDigest>,
    key: VerifyingKey,
}

impl Verifier {
    fn from_public_key(key: SubjectPublicKeyInfoRef) -> LoggedResult<Verifier> {
        let digest: Box<dyn DynDigest>;
        let key = if let Ok(rsa) = RsaPublicKey::try_from(key.clone()) {
            digest = Box::<Sha256>::default();
            VerifyingKey::SHA256withRSA(RsaVerifyingKey::<Sha256>::new(rsa))
        } else if let Ok(ec) = P256VerifyingKey::try_from(key.clone()) {
            digest = Box::<Sha256>::default();
            VerifyingKey::SHA256withECDSA(ec)
        } else if let Ok(ec) = P384VerifyingKey::try_from(key.clone()) {
            digest = Box::<Sha384>::default();
            VerifyingKey::SHA384withECDSA(ec)
        } else if let Ok(ec) = P521VerifyingKey::try_from(key.clone()) {
            digest = Box::<Sha512>::default();
            VerifyingKey::SHA521withECDSA(ec)
        } else {
            return Err(log_err!("Unsupported private key"));
        };
        Ok(Verifier { digest, key })
    }

    fn update(&mut self, data: &[u8]) {
        self.digest.update(data)
    }

    fn verify(mut self, signature: &[u8]) -> LoggedResult<()> {
        let hash = self.digest.finalize_reset();
        match &self.key {
            VerifyingKey::SHA256withRSA(key) => {
                let sig = RsaSignature::try_from(signature)?;
                key.verify_prehash(hash.as_ref(), &sig).log()
            }
            VerifyingKey::SHA256withECDSA(key) => {
                let sig = P256Signature::from_slice(signature)?;
                key.verify_prehash(hash.as_ref(), &sig).log()
            }
            VerifyingKey::SHA384withECDSA(key) => {
                let sig = P384Signature::from_slice(signature)?;
                key.verify_prehash(hash.as_ref(), &sig).log()
            }
            VerifyingKey::SHA521withECDSA(key) => {
                let sig = P521Signature::from_slice(signature)?;
                key.verify_prehash(hash.as_ref(), &sig).log()
            }
        }
    }
}

struct Signer {
    digest: Box<dyn DynDigest>,
    key: SigningKey,
}

impl Signer {
    fn from_private_key(key: &[u8]) -> LoggedResult<Signer> {
        let digest: Box<dyn DynDigest>;
        let key = match RsaPrivateKey::from_pkcs8_der(key) {
            Ok(rsa) => {
                digest = Box::<Sha256>::default();
                SigningKey::SHA256withRSA(RsaSigningKey::<Sha256>::new(rsa))
            }
            _ => match P256SigningKey::from_pkcs8_der(key) {
                Ok(ec) => {
                    digest = Box::<Sha256>::default();
                    SigningKey::SHA256withECDSA(ec)
                }
                _ => match P384SigningKey::from_pkcs8_der(key) {
                    Ok(ec) => {
                        digest = Box::<Sha384>::default();
                        SigningKey::SHA384withECDSA(ec)
                    }
                    _ => match P521SigningKey::from_pkcs8_der(key) {
                        Ok(ec) => {
                            digest = Box::<Sha512>::default();
                            SigningKey::SHA521withECDSA(ec)
                        }
                        _ => {
                            return Err(log_err!("Unsupported private key"));
                        }
                    },
                },
            },
        };
        Ok(Signer { digest, key })
    }

    fn update(&mut self, data: &[u8]) {
        self.digest.update(data)
    }

    fn sign(mut self) -> LoggedResult<Vec<u8>> {
        let hash = self.digest.finalize_reset();
        let v = match &self.key {
            SigningKey::SHA256withRSA(key) => {
                let sig: RsaSignature = key.sign_prehash(hash.as_ref())?;
                sig.to_vec()
            }
            SigningKey::SHA256withECDSA(key) => {
                let sig: P256Signature = key.sign_prehash(hash.as_ref())?;
                sig.to_vec()
            }
            SigningKey::SHA384withECDSA(key) => {
                let sig: P384Signature = key.sign_prehash(hash.as_ref())?;
                sig.to_vec()
            }
            SigningKey::SHA521withECDSA(key) => {
                let sig: P521Signature = key.sign_prehash(hash.as_ref())?;
                sig.to_vec()
            }
        };
        Ok(v)
    }
}

/*
 * BootSignature ::= SEQUENCE {
 *     formatVersion ::= INTEGER,
 *     certificate ::= Certificate,
 *     algorithmIdentifier ::= SEQUENCE {
 *         algorithm OBJECT IDENTIFIER,
 *         parameters ANY DEFINED BY algorithm OPTIONAL
 *     },
 *     authenticatedAttributes ::= SEQUENCE {
 *         target CHARACTER STRING,
 *         length INTEGER
 *     },
 *     signature ::= OCTET STRING
 * }
 */

#[derive(Sequence)]
struct AuthenticatedAttributes {
    target: PrintableString,
    length: u64,
}

#[derive(Sequence)]
struct BootSignature {
    format_version: i32,
    certificate: Certificate,
    algorithm_identifier: AlgorithmIdentifier<Any>,
    authenticated_attributes: AuthenticatedAttributes,
    signature: OctetString,
}

impl BootSignature {
    fn verify(self, payload: &[u8]) -> LoggedResult<()> {
        if self.authenticated_attributes.length as usize != payload.len() {
            return Err(log_err!("Invalid image size"));
        }
        let mut verifier = Verifier::from_public_key(
            self.certificate
                .tbs_certificate()
                .subject_public_key_info()
                .owned_to_ref(),
        )?;
        verifier.update(payload);
        let attr = self.authenticated_attributes.to_der()?;
        verifier.update(attr.as_slice());
        verifier.verify(self.signature.as_bytes())?;
        Ok(())
    }
}

pub fn verify_boot_image(img: &BootImage, cert: *const c_char) -> bool {
    let res: LoggedResult<()> = try {
        let tail = img.tail();
        // Don't use BootSignature::from_der because tail might have trailing zeros
        let mut reader = SliceReader::new(tail)?;
        let mut sig = BootSignature::decode(&mut reader)?;
        match unsafe { Utf8CStr::from_ptr(cert) } {
            Ok(s) => {
                let pem = MappedFile::open(s)?;
                sig.certificate = Certificate::from_pem(pem)?;
            }
            Err(StrErr::NullPointerError) => {}
            Err(e) => Err(e)?,
        };
        sig.verify(img.payload())?;
    };
    res.is_ok()
}

enum Bytes {
    Mapped(MappedFile),
    Slice(&'static [u8]),
}

impl AsRef<[u8]> for Bytes {
    fn as_ref(&self) -> &[u8] {
        match self {
            Bytes::Mapped(m) => m.as_ref(),
            Bytes::Slice(s) => s,
        }
    }
}

const VERITY_PEM: &[u8] = include_bytes!("../../../tools/keys/verity.x509.pem");
const VERITY_PK8: &[u8] = include_bytes!("../../../tools/keys/verity.pk8");

pub fn sign_boot_image(
    payload: &[u8],
    name: *const c_char,
    cert: *const c_char,
    key: *const c_char,
) -> Vec<u8> {
    let res: LoggedResult<Vec<u8>> = try {
        // Process arguments
        let name = unsafe { Utf8CStr::from_ptr(name) }?;
        let cert = match unsafe { Utf8CStr::from_ptr(cert) } {
            Ok(s) => Bytes::Mapped(MappedFile::open(s)?),
            Err(StrErr::NullPointerError) => Bytes::Slice(VERITY_PEM),
            Err(e) => Err(e)?,
        };
        let key = match unsafe { Utf8CStr::from_ptr(key) } {
            Ok(s) => Bytes::Mapped(MappedFile::open(s)?),
            Err(StrErr::NullPointerError) => Bytes::Slice(VERITY_PK8),
            Err(e) => Err(e)?,
        };

        // Parse cert and private key
        let cert = Certificate::from_pem(cert)?;
        let mut signer = Signer::from_private_key(key.as_ref())?;

        // Sign image
        let attr = AuthenticatedAttributes {
            target: PrintableString::new(name.as_bytes())?,
            length: payload.len() as u64,
        };
        signer.update(payload);
        signer.update(attr.to_der()?.as_slice());
        let sig = signer.sign()?;

        // Create BootSignature DER
        let alg_id = cert.signature_algorithm().clone();
        let sig = BootSignature {
            format_version: 1,
            certificate: cert,
            algorithm_identifier: alg_id,
            authenticated_attributes: attr,
            signature: OctetString::new(sig)?,
        };
        sig.to_der()?
    };
    res.unwrap_or_default()
}
