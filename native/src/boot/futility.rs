// ChromeOS verified boot kernel signing, ported from vboot_reference futility.
//
// The output is byte-for-byte identical to:
//
// echo > empty
// futility vbutil_kernel --pack <out> --keyblock kernel.keyblock \
//   --signprivate kernel_data_key.vbprivk --version 1 --vmlinuz <img> \
//   --config empty --arch arm --bootloader empty --flags 0x1
//
// Output layout:
//
// +--------------------------------+
// | keyblock                       | \
// +--------------------------------+  > vblock, padded to 64 KiB
// | kernel preamble + padding      | /
// +--------------------------------+
// | kernel image, aligned to 4K    | \
// | config (4K, empty)             |  |
// | params (4K, unused on ARM)     |  > kernel blob
// | bootloader stub (4K)           | /
// +--------------------------------+

use crate::sign::sha256_hash;
use base::{LoggedResult, log_err};
use bytemuck::{Pod, Zeroable, bytes_of};
use rsa::RsaPrivateKey;
use rsa::pkcs1::DecodeRsaPrivateKey;
use rsa::pkcs1v15::{Signature, SigningKey};
use rsa::signature::SignatureEncoding;
use rsa::signature::hazmat::PrehashSigner;
use sha2::Sha256;
use std::mem::{offset_of, size_of};

const KEYBLOCK: &[u8] = include_bytes!("../../../tools/keys/kernel.keyblock");
// 8 bytes of algorithm ID, followed by a PKCS#1 DER encoded RSA private key
const PRIVKEY: &[u8] = include_bytes!("../../../tools/keys/kernel_data_key.vbprivk");
const PRIVKEY_HDR_SIZE: usize = 8;
// vb2_crypto_algorithm: VB2_ALG_RSA2048_SHA256
const ALG_RSA2048_SHA256: u8 = 4;
const RSA2048_SIG_SIZE: usize = 256;

const CROS_ALIGN: usize = 4096;
const CROS_CONFIG_SIZE: usize = 4096;
const CROS_PARAMS_SIZE: usize = 4096;
// --kloadaddr (default: CROS_32BIT_ENTRY_ADDR)
const CROS_BODY_LOAD_ADDR: u64 = 0x100000;
// --pad (default)
const CROS_VBLOCK_SIZE: usize = 65536;
// --version 1
const CROS_KERNEL_VERSION: u32 = 1;
// --flags 0x1
const CROS_KERNEL_FLAGS: u32 = 1;
// --bootloader empty (created with `echo > empty`)
const CROS_BOOTLOADER: &[u8] = b"\n";

const PREAMBLE_HEADER_VERSION_MAJOR: u32 = 2;
const PREAMBLE_HEADER_VERSION_MINOR: u32 = 2;
const PREAMBLE_HDR_SIZE: usize = size_of::<VbKernelPreamble>();
// The preamble signature covers the header and the body signature
const PREAMBLE_SIGNED_SIZE: usize = PREAMBLE_HDR_SIZE + RSA2048_SIG_SIZE;
// Pad the preamble so that keyblock + preamble fills up the whole vblock
const PREAMBLE_SIZE: usize = CROS_VBLOCK_SIZE - KEYBLOCK.len();

const _: () = assert!(PRIVKEY[0] == ALG_RSA2048_SHA256);
const _: () = assert!(PREAMBLE_HDR_SIZE == 116);
const _: () = assert!(PREAMBLE_SIZE >= PREAMBLE_SIGNED_SIZE + RSA2048_SIG_SIZE);

// struct vb2_signature
#[repr(C, packed)]
#[derive(Pod, Zeroable, Clone, Copy)]
struct VbSignature {
    // Offset of signature data from the start of this struct
    sig_offset: u32,
    reserved0: u32,
    sig_size: u32,
    reserved1: u32,
    // Size of the data block which was signed
    data_size: u32,
    reserved2: u32,
}

// struct vb2_kernel_preamble
#[repr(C, packed)]
#[derive(Pod, Zeroable, Clone, Copy)]
struct VbKernelPreamble {
    preamble_size: u32,
    reserved0: u32,
    preamble_signature: VbSignature,
    header_version_major: u32,
    header_version_minor: u32,
    kernel_version: u32,
    reserved1: u32,
    body_load_address: u64,
    bootloader_address: u64,
    bootloader_size: u32,
    reserved2: u32,
    body_signature: VbSignature,
    vmlinuz_header_address: u64,
    vmlinuz_header_size: u32,
    reserved3: u32,
    flags: u32,
}

fn align_up(val: usize, alignment: usize) -> usize {
    val.div_ceil(alignment) * alignment
}

fn sign_rsa2048_sha256(key: &SigningKey<Sha256>, data: &[u8]) -> LoggedResult<Vec<u8>> {
    let mut hash = [0u8; 32];
    sha256_hash(data, &mut hash);
    let sig: Signature = key.sign_prehash(&hash)?;
    let sig = sig.to_vec();
    if sig.len() != RSA2048_SIG_SIZE {
        return log_err!("Invalid signature size");
    }
    Ok(sig)
}

pub fn sign_chromeos_image(kernel: &[u8]) -> LoggedResult<Vec<u8>> {
    if kernel.is_empty() {
        return log_err!("Empty kernel image");
    }

    let key = RsaPrivateKey::from_pkcs1_der(&PRIVKEY[PRIVKEY_HDR_SIZE..])?;
    let key = SigningKey::<Sha256>::new(key);

    // Kernel blob layout
    let config_off = align_up(kernel.len(), CROS_ALIGN);
    let bootloader_off = config_off + CROS_CONFIG_SIZE + CROS_PARAMS_SIZE;
    let bootloader_size = align_up(CROS_BOOTLOADER.len(), CROS_ALIGN);
    let blob_size = align_up(bootloader_off + bootloader_size, CROS_ALIGN);

    let mut out = vec![0u8; KEYBLOCK.len() + PREAMBLE_SIZE + blob_size];
    let (keyblock, rest) = out.split_at_mut(KEYBLOCK.len());
    let (preamble, blob) = rest.split_at_mut(PREAMBLE_SIZE);

    // Keyblock
    keyblock.copy_from_slice(KEYBLOCK);

    // Kernel blob
    blob[..kernel.len()].copy_from_slice(kernel);
    blob[bootloader_off..][..CROS_BOOTLOADER.len()].copy_from_slice(CROS_BOOTLOADER);
    let body_sig = sign_rsa2048_sha256(&key, blob)?;

    // Kernel preamble
    let hdr = VbKernelPreamble {
        preamble_size: PREAMBLE_SIZE as u32,
        preamble_signature: VbSignature {
            sig_offset: (PREAMBLE_SIGNED_SIZE - offset_of!(VbKernelPreamble, preamble_signature))
                as u32,
            sig_size: RSA2048_SIG_SIZE as u32,
            data_size: PREAMBLE_SIGNED_SIZE as u32,
            ..Zeroable::zeroed()
        },
        header_version_major: PREAMBLE_HEADER_VERSION_MAJOR,
        header_version_minor: PREAMBLE_HEADER_VERSION_MINOR,
        kernel_version: CROS_KERNEL_VERSION,
        body_load_address: CROS_BODY_LOAD_ADDR,
        bootloader_address: CROS_BODY_LOAD_ADDR + bootloader_off as u64,
        bootloader_size: bootloader_size as u32,
        body_signature: VbSignature {
            sig_offset: (PREAMBLE_HDR_SIZE - offset_of!(VbKernelPreamble, body_signature)) as u32,
            sig_size: RSA2048_SIG_SIZE as u32,
            data_size: u32::try_from(blob_size)?,
            ..Zeroable::zeroed()
        },
        flags: CROS_KERNEL_FLAGS,
        ..Zeroable::zeroed()
    };
    preamble[..PREAMBLE_HDR_SIZE].copy_from_slice(bytes_of(&hdr));
    preamble[PREAMBLE_HDR_SIZE..PREAMBLE_SIGNED_SIZE].copy_from_slice(&body_sig);
    let preamble_sig = sign_rsa2048_sha256(&key, &preamble[..PREAMBLE_SIGNED_SIZE])?;
    preamble[PREAMBLE_SIGNED_SIZE..][..RSA2048_SIG_SIZE].copy_from_slice(&preamble_sig);

    Ok(out)
}
