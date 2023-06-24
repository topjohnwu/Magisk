use digest::DynDigest;
use sha1::Sha1;
use sha2::Sha256;

pub struct SHA {
    hasher: Box<dyn DynDigest>,
}

impl SHA {
    pub fn update(&mut self, data: &[u8]) {
        self.hasher.update(data);
    }

    pub fn finalize(&mut self) -> Vec<u8> {
        self.hasher.finalize_reset().to_vec()
    }
}

pub fn get_sha(use_sha1: bool) -> Box<SHA> {
    Box::new(SHA {
        hasher: if use_sha1 {
            Box::new(Sha1::default())
        } else {
            Box::new(Sha256::default())
        },
    })
}

pub fn sha_digest(data: &[u8], use_sha1: bool) -> Vec<u8> {
    let mut sha = get_sha(use_sha1);
    sha.update(data);
    sha.finalize()
}
