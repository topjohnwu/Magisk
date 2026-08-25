//! ARM 32-bit Linux zImage Parsing and Piggy Resizing / Relocation
//!
//! # Background & Overview
//!
//! A 32-bit ARM Linux kernel image (`zImage`) is a self-extracting executable wrapper containing:
//! 1. `head`: Decompressor startup code (`head.o`), runtime decompression routines (`misc.o`,
//!    `decompress.o`), and the `piggy.o` wrapper header. It starts with the standard ARM `zimage_hdr`
//!    (magic `0x016f2818` at offset `0x24`, start/end execution addresses at offsets `0x28`/`0x2c`).
//! 2. `piggy`: The compressed kernel payload (`vmlinux.bin.gz`, `.xz`, `.lz4`, etc.), with an
//!    optional appended 4-byte LE uncompressed size (`size_append`) for non-gzip algorithms.
//! 3. `tail`: Trailing data following the compressed piggy, containing padding, the decompressor's
//!    Global Offset Table (`.got` from `_got_start` to `_got_end`), decompressor stack, and optional
//!    appended Device Tree Blobs (DTBs).
//!
//! # The Resizing & Relocation Problem
//!
//! When modifying or patching the kernel (e.g., patching kernel instructions or data),
//! the re-compressed payload size almost certainly changes. In traditional repack workflows, resizing
//! the `piggy` blob breaks the zImage because the decompressor code in `head` and the data in `tail`
//! contain hardcoded absolute and relative offsets that assume a fixed `piggy` length.
//!
//! Rather than requiring exact compression size matches (e.g., brute-forcing compression parameters or
//! using zopfli) or needing full kernel source and toolchains to rebuild the wrapper, we parse the
//! internal metadata of the zImage and dynamically patch all relocated offsets:
//!
//! 1. **`zimage_hdr.end`**:
//!    The total image end address in the zImage header (`offset 0x2c`) is adjusted by `+ delta_got`.
//!
//! 2. **Header Relocation Tables**:
//!    - `.Linflated_image_size_offset`: The pointer offset locating the uncompressed image size.
//!    - `LC0` Table: Position-independent relocation table in `arch/arm/boot/compressed/head.S` containing
//!      addresses for `_start`, `_got_start`, `_got_end`, `_edata`, `__bss_start`, and `_end`. Any entry
//!      pointing to `orig_piggy_end` is updated to `new_piggy_end`, and any entry pointing past `head`
//!      is adjusted by `+ delta_got`.
//!    - `LC1` Table: Additional relocation table entries in newer kernels adjusted by `+ delta_got`.
//!    - `_magic_table` (`.table` / `0x45454545` or `0x45455358`): Tagged kernel size entry adjusted.
//!
//! 3. **Trailing GOT & Stack (`new_tail`)**:
//!    - The `tail` is realigned to 4 bytes after `new_piggy_end` with zero padding.
//!    - Internal GOT pointers matching `orig_piggy_end` or referencing GOT symbols `>= orig_got_start - 4`
//!      are shifted by `+ delta_got`.
//!
//! 4. **R_ARM_GOTPC Literals in Decompressor `.text` (`new_head`)**:
//!    - Decompressor code calculates the GOT base address relative to PC using patterns like:
//!      `ldr rX, [pc, #imm]` followed by `add rY, pc, rX`.
//!    - The literal loaded from the constant pool represents `_GLOBAL_OFFSET_TABLE_ - (. + 8)`.
//!      Because the GOT in `tail` shifts by `delta_got` while `.text` in `head` stays fixed, these
//!      literal offsets must be adjusted by `+ delta_got`.
//!
//! # Reference
//!
//! - "Modifying Embedded Filesystems in ARM Linux zImages" by jamchamb:
//!   <https://jamchamb.net/2022/01/02/modify-vmlinuz-arm.html>

use crate::ffi::{FileFormat, ZImage};
use crate::format::check_fmt;

#[inline(always)]
fn read_u32(buf: &[u8], off: usize) -> u32 {
    let end = match off.checked_add(4) {
        Some(e) => e,
        None => return 0,
    };
    buf.get(off..end)
        .and_then(|s| s.try_into().ok())
        .map(u32::from_le_bytes)
        .unwrap_or(0)
}

#[inline(always)]
fn write_u32(buf: &mut [u8], off: usize, val: u32) {
    let end = match off.checked_add(4) {
        Some(e) => e,
        None => return,
    };
    if let Some(slice) = buf.get_mut(off..end) {
        slice.copy_from_slice(&val.to_le_bytes());
    }
}

/// Validates whether a candidate piggy_end offset is structurally consistent with the zImage layout.
/// Checks that the candidate lies strictly past the header and within total kernel bounds.
/// If GOT boundaries are known from the LC0 table, verifies that the candidate's 4-byte aligned
/// boundary matches the start of the Global Offset Table (got_start), as defined in vmlinux.lds.S:
///     . = ALIGN(4);
///     _got_start = .;
fn is_valid_piggy_end(kernel: &[u8], hdr_sz: usize, candidate_end: usize, got_start: usize) -> bool {
    if candidate_end <= hdr_sz.saturating_add(4) || candidate_end > kernel.len() {
        return false;
    }
    // When got_start is known from LC0, candidate_end aligned to 4 bytes must equal got_start
    if got_start != 0 && (candidate_end.saturating_add(3) & !3) != got_start {
        return false;
    }
    true
}

impl<'a> ZImage<'a> {
    pub fn parse(zimg: &'a [u8]) -> cxx::UniquePtr<Self> {
        if let Some(z) = Self::parse_impl(zimg) {
            cxx::UniquePtr::new(z)
        } else {
            cxx::UniquePtr::null()
        }
    }

    fn parse_impl(zimg: &'a [u8]) -> Option<Self> {
        // Step 1: Validate 32-bit ARM zImage header (magic 0x016f2818 at offset 0x24)
        if zimg.len() < 0x28 {
            return None;
        }

        let magic = read_u32(zimg, 0x24);
        if magic != 0x016f2818 {
            return None;
        }

        let start = read_u32(zimg, 0x28);
        let end = read_u32(zimg, 0x2C);
        let table_magic = read_u32(zimg, 0x34);
        let table_offset = read_u32(zimg, 0x38);

        // Step 2: Locate the start of compressed piggy payload by scanning forward from 0x28
        let mut piggy_ptr = None;
        for curr in 0x28..zimg.len() {
            if check_fmt(&zimg[curr..]) != FileFormat::UNKNOWN {
                piggy_ptr = Some(curr);
                break;
            }
        }

        let piggy_off = match piggy_ptr {
            Some(off) => off,
            None => {
                eprintln!("! Could not find zImage piggy, keeping raw kernel");
                return None;
            }
        };

        let head = &zimg[..piggy_off];
        let fmt = check_fmt(&zimg[piggy_off..]);

        // Step 3: Scan for the LC0 position table (arch/arm/boot/compressed/head.S)
        // LC0 contains position-independent runtime pointers: _start, _got_start, _got_end, _edata, etc.
        // We match: p0 == off + start (pointer to _start), with p1 >= head.len() and p2 >= p1
        let mut off_lc0 = 0u32;
        let mut got_start = 0usize;
        let mut got_end = 0usize;
        if head.len() >= 32 {
            for off in (0x20..=head.len() - 32).step_by(4) {
                let p0 = read_u32(head, off);
                if p0 == (off as u32).wrapping_add(start) {
                    let p1 = read_u32(head, off + 4);
                    let p2 = read_u32(head, off + 8);
                    if p1 >= head.len() as u32 && p2 >= p1 {
                        off_lc0 = off as u32;
                        let lc0_1 = read_u32(head, off + 4);
                        let lc0_4 = read_u32(head, off + 16);
                        let lc0_5 = read_u32(head, off + 20);
                        let lc0_6 = read_u32(head, off + 24);
                        if lc0_5 <= lc0_6 && lc0_6 <= lc0_1 && lc0_5 > start {
                            got_start = (lc0_5 - start) as usize;
                            got_end = (lc0_6 - start) as usize;
                        } else if lc0_4 <= lc0_5 && lc0_5 <= lc0_1 && lc0_4 > start {
                            got_start = (lc0_4 - start) as usize;
                            got_end = (lc0_5 - start) as usize;
                        }
                        break;
                    }
                }
            }
        }

        let total_zimage_sz = if end > start && (end - start) as usize <= zimg.len() {
            (end - start) as usize
        } else {
            zimg.len()
        };

        let mut piggy_end = zimg.len();
        let mut off_table = 0u32;

        // Step 4: Determine piggy_end using a 3-tier heuristic strategy

        // Strategy 4.1: Check .table (_magic_table) TagKernelSize (0x5a534c4b) if present at offset 0x38.
        // The table is tagged with 0x45454545 ("EEEE"), 0x45455358 ("XSEE"), or 0x20425444 ("DTB ").
        if (table_magic == 0x45454545 || table_magic == 0x45455358 || table_magic == 0x20425444)
            && table_offset > 0
            && (table_offset as usize) < head.len()
        {
            let mut tbl_off = table_offset as usize;
            while tbl_off + 8 <= head.len() {
                let num_words = read_u32(head, tbl_off) as usize;
                if num_words == 0 {
                    break;
                }
                let tag = read_u32(head, tbl_off + 4);
                if tag == 0x5a534c4b
                    && num_words >= 3
                    && tbl_off
                        .checked_add(num_words.wrapping_mul(4))
                        .is_some_and(|end| end <= head.len())
                {
                    let piggy_size_addr = read_u32(head, tbl_off + 8);
                    let candidate_end = (piggy_size_addr.wrapping_sub(start).wrapping_add(4)) as usize;
                    if is_valid_piggy_end(zimg, head.len(), candidate_end, got_start) {
                        piggy_end = candidate_end;
                        off_table = (tbl_off + 8) as u32;
                        break;
                    }
                }
                if let Some(next) = tbl_off.checked_add(num_words.wrapping_mul(4)) {
                    tbl_off = next;
                } else {
                    break;
                }
            }
        }

        // Strategy 4.2 (Fallback): Derive GOT bounds from LC0 entries and scan for pointer to input_data_end
        if piggy_end == zimg.len() {
            let min_piggy_end = if total_zimage_sz > 0x200 {
                total_zimage_sz - 0x200
            } else {
                head.len()
            };
            if got_start >= head.len() && got_end <= total_zimage_sz && got_start <= got_end {
                for off in (got_start..=got_end.min(total_zimage_sz.saturating_sub(4))).step_by(4) {
                    let val = (read_u32(zimg, off).wrapping_sub(start)) as usize;
                    if val > min_piggy_end && val < total_zimage_sz {
                        piggy_end = val;
                        break;
                    }
                }
            }

            // Strategy 4.3 (Fallback): Scan the last 16 dwords of the zImage tail for input_data_end pointer
            if piggy_end == zimg.len() && total_zimage_sz > 64 {
                let min_tail_end = if total_zimage_sz > 0xFF {
                    total_zimage_sz - 0xFF
                } else {
                    head.len()
                };
                for i in 1..=16 {
                    let off = total_zimage_sz.saturating_sub(i * 4);
                    let val = (read_u32(zimg, off).wrapping_sub(start)) as usize;
                    if val > min_tail_end && val < total_zimage_sz {
                        piggy_end = val;
                        break;
                    }
                }
            }
        }

        // Step 5: Locate .Linflated_image_size_offset matching known piggy_end
        // This is a literal in head matching: off + val + 4 == piggy_end
        let mut off_inflated_size = 0u32;
        if piggy_end != zimg.len() && head.len() >= 4 {
            for off in (0x20..=head.len() - 4).step_by(4) {
                let val = read_u32(head, off);
                if (off as u32).wrapping_add(val).wrapping_add(4) == piggy_end as u32 {
                    off_inflated_size = off as u32;
                    break;
                }
            }
        }

        if piggy_end == zimg.len() || piggy_end <= head.len() {
            eprintln!("! Could not find end of zImage piggy, keeping raw kernel");
            return None;
        }

        let piggy = &zimg[head.len()..piggy_end];

        // Step 6: Scan for LC1 relocation table in newer kernels (where p1 + off + start == end)
        let mut off_lc1 = 0u32;
        if head.len() >= 8 {
            for off in (0x20..=head.len() - 8).step_by(4) {
                let p1 = read_u32(head, off + 4);
                if p1.wrapping_add(off as u32).wrapping_add(start) == end {
                    off_lc1 = off as u32;
                    break;
                }
            }
        }

        let tail = &zimg[piggy_end..];

        if piggy.is_empty() || (fmt != FileFormat::GZIP && piggy.len() < 4) {
            return None;
        }

        // Step 7: Construct zero-copy ZImage struct slicing head, piggy, and tail
        Some(ZImage {
            head,
            piggy,
            tail,
            fmt,
            off_inflated_size,
            off_lc0,
            off_lc1,
            off_table,
        })
    }

    pub fn new_head(&self, payload_sz: usize) -> Vec<u8> {
        let mut head_stub = self.head.to_vec();
        if head_stub.is_empty() || payload_sz == 0 || payload_sz == self.piggy.len() {
            return head_stub;
        }

        // Step 1: Calculate offset delta for GOT and trailing sections based on 4-byte realignment
        let start = read_u32(self.head, 0x28);
        let orig_piggy_end = (self.head.len() as u32).wrapping_add(self.piggy.len() as u32).wrapping_add(start);
        let orig_got_start = (orig_piggy_end + 3) & !3;
        let new_piggy_end = (self.head.len() as u32).wrapping_add(payload_sz as u32).wrapping_add(start);
        let new_got_start = (new_piggy_end + 3) & !3;
        let delta_got = new_got_start.wrapping_sub(orig_got_start);

        // Step 2: Patch zimage_hdr->end (offset 0x2C) with new total image end address
        let end = read_u32(&head_stub, 0x2C);
        write_u32(&mut head_stub, 0x2C, end.wrapping_add(delta_got));

        // Step 3: Patch .Linflated_image_size_offset pointer table entry
        if self.off_inflated_size != 0 {
            let off = self.off_inflated_size as usize;
            let val = read_u32(&head_stub, off);
            write_u32(&mut head_stub, off, val.wrapping_add(delta_got));
        }

        // Step 4: Patch the LC0 position table (arch/arm/boot/compressed/head.S)
        // Entry pointing to input_data_end - 4 (uncompressed size word) is updated to new_piggy_end - 4.
        // Entries pointing into tail (>= head.len() + start) are shifted by + delta_got.
        if self.off_lc0 != 0 {
            let off = self.off_lc0 as usize;
            for i in 1..8 {
                let entry_off = off + i * 4;
                let val = read_u32(&head_stub, entry_off);
                if val == orig_piggy_end.wrapping_sub(4) {
                    write_u32(&mut head_stub, entry_off, new_piggy_end.wrapping_sub(4));
                } else if val >= (self.head.len() as u32).wrapping_add(start) {
                    write_u32(&mut head_stub, entry_off, val.wrapping_add(delta_got));
                }
            }
        }

        // Step 5: Patch LC1 relocation table in newer kernels
        if self.off_lc1 != 0 {
            let off = self.off_lc1 as usize;
            let val0 = read_u32(&head_stub, off);
            let val1 = read_u32(&head_stub, off + 4);
            write_u32(&mut head_stub, off, val0.wrapping_add(delta_got));
            write_u32(&mut head_stub, off + 4, val1.wrapping_add(delta_got));
        }

        // Step 6: Patch .table (_magic_table) TagKernelSize entry
        if self.off_table != 0 {
            let off = self.off_table as usize;
            let val = read_u32(&head_stub, off);
            write_u32(&mut head_stub, off, val.wrapping_add(delta_got));
        }

        // Step 7: Disassemble and patch R_ARM_GOTPC PC-relative literal pool entries in .text
        // In -fPIC, ARM code accesses GOT via:
        //   ldr rX, [pc, #imm]   ; loads (_GLOBAL_OFFSET_TABLE_ - (. + 8)) from constant pool
        //   add rY, pc, rX       ; rY = &GOT
        // When the tail shifts by delta_got, the constant pool literals must be adjusted by + delta_got.

        // Locate GOT bounds from LC0
        let mut got_start = 0u32;
        let mut got_end = 0u32;
        if self.off_lc0 != 0 {
            let off = self.off_lc0 as usize;
            let lc0_1 = read_u32(self.head, off + 4);
            let lc0_4 = read_u32(self.head, off + 16);
            let lc0_5 = read_u32(self.head, off + 20);
            let lc0_6 = read_u32(self.head, off + 24);
            if lc0_5 <= lc0_6 && lc0_6 <= lc0_1 && lc0_5 > start {
                got_start = lc0_5;
                got_end = lc0_6;
            } else if lc0_4 <= lc0_5 && lc0_5 <= lc0_1 && lc0_4 > start {
                got_start = lc0_4;
                got_end = lc0_5;
            }
        }

        if got_start != 0 && got_end != 0 {
            // Scan for ARM: add rd, pc, rm (opcode: cond 0000 100S rn=1111 rd 0000 0000 rm)
            for off in (0..=head_stub.len().saturating_sub(4)).step_by(4) {
                let insn = read_u32(&head_stub, off);
                if (insn & 0x0fe00000) == 0x00800000 && ((insn >> 16) & 0xf) == 0xf {
                    let rm = insn & 0xf;
                    // Look back up to 128 bytes for preceding: ldr rt, [pc, #imm]
                    let lookback = off.saturating_sub(128);
                    for prev_off in (lookback..off).step_by(4) {
                        let prev_insn = read_u32(&head_stub, prev_off);
                        if (prev_insn & 0x0f7f0000) == 0x051f0000 {
                            let rt = (prev_insn >> 12) & 0xf;
                            if rt == rm {
                                let u = ((prev_insn >> 23) & 1) != 0;
                                let imm = (prev_insn & 0xfff) as usize;
                                let lit_addr = if u {
                                    (prev_off + 8).checked_add(imm)
                                } else {
                                    (prev_off + 8).checked_sub(imm)
                                };
                                if let Some(lit_addr) = lit_addr
                                    && lit_addr + 4 <= head_stub.len()
                                {
                                    let val = read_u32(&head_stub, lit_addr);
                                    let target = (off as u32 + 8).wrapping_add(val).wrapping_add(start);
                                    // Verify that computed target falls within GOT range
                                    if target >= got_start.saturating_sub(0x20)
                                        && target <= got_end.wrapping_add(0x20)
                                    {
                                        write_u32(&mut head_stub, lit_addr, val.wrapping_add(delta_got));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        head_stub
    }

    pub fn new_tail(&self, payload_sz: usize) -> Vec<u8> {
        if self.tail.is_empty() || payload_sz == 0 || payload_sz == self.piggy.len() {
            return self.tail.to_vec();
        }

        let start = read_u32(self.head, 0x28);
        let orig_piggy_end = (self.head.len() as u32).wrapping_add(self.piggy.len() as u32).wrapping_add(start);
        let orig_got_start = (orig_piggy_end + 3) & !3;
        let orig_pad_sz = (orig_got_start - orig_piggy_end) as usize;

        let new_piggy_end = (self.head.len() as u32).wrapping_add(payload_sz as u32).wrapping_add(start);
        let new_got_start = (new_piggy_end + 3) & !3;
        let new_pad_sz = (new_got_start - new_piggy_end) as usize;
        let delta_got = new_got_start.wrapping_sub(orig_got_start);

        // Step 1: Re-pad leading gap before the GOT with zeros to maintain 4-byte alignment
        let mut result = vec![0u8; new_pad_sz];
        if orig_pad_sz < self.tail.len() {
            result.extend_from_slice(&self.tail[orig_pad_sz..]);
        }

        // Step 2: Relocate internal GOT pointers (arch/arm/boot/compressed/vmlinux.lds.S)
        // Pointers matching orig_piggy_end (input_data_end) are updated to new_piggy_end.
        // Pointers to GOT symbols (>= orig_got_start - 4) are shifted by + delta_got.
        if self.off_lc0 != 0 {
            for i in (new_pad_sz..=result.len().saturating_sub(4)).step_by(4) {
                let mut val = read_u32(&result, i);
                if val == orig_piggy_end {
                    val = new_piggy_end;
                    write_u32(&mut result, i, val);
                } else if val >= orig_got_start.wrapping_sub(4) {
                    val = val.wrapping_add(delta_got);
                    write_u32(&mut result, i, val);
                }
            }
        }

        result
    }
}
