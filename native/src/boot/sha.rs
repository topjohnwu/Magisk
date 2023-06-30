use digest::DynDigest;
use sha1::Sha1;
use sha2::Sha256;

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
