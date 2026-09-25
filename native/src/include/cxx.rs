// Adapt the pinned CXX generator's output without modifying the dependency.
// C ABI definitions call C++; C++ definitions call Rust. Runtime helpers are
// shared, and private C ABI declarations may be needed by either direction.

use std::collections::BTreeSet;

pub struct Source {
    pub preamble: String,
    pub helpers: String,
    pub definitions: String,
}

pub struct Bridge {
    pub declarations: String,
    pub rust_api: Source,
    pub cxx_wrappers: Source,
}

type Result<T> = std::result::Result<T, String>;

pub fn split(code: cxx_gen::GeneratedCode) -> Result<Bridge> {
    let header = String::from_utf8(code.header).map_err(|e| e.to_string())?;
    let implementation = String::from_utf8(code.implementation).map_err(|e| e.to_string())?;
    let forward = header
        .lines()
        .map(str::trim)
        .filter(|line| {
            line.ends_with(';')
                && (line.starts_with("struct ")
                    || line.starts_with("enum class ")
                    || line.starts_with("using "))
        })
        .collect::<BTreeSet<_>>();
    let mut namespaces = Vec::new();
    let mut offset = 0;
    let mut boundary = None;
    for line in header.split_inclusive('\n') {
        let trimmed = line.trim();
        if trimmed.starts_with("namespace ") && trimmed.ends_with('{') {
            namespaces.push(offset);
        } else if trimmed.starts_with('}') {
            namespaces.pop();
        }
        if forward.contains(trimmed) {
            let start = namespaces.first().copied().unwrap_or(offset);
            boundary = Some(&header[start..offset + line.trim_end().len()]);
            break;
        }
        offset += line.len();
    }
    let boundary = boundary.ok_or("CXX output has no application declarations")?;
    let start = implementation
        .find(boundary)
        .ok_or("CXX implementation declaration boundary changed")?;
    let mut preamble = String::new();
    let mut helpers = String::new();
    for line in implementation[..start].lines() {
        let target = if line.starts_with("#include ") {
            &mut preamble
        } else {
            &mut helpers
        };
        target.push_str(line);
        target.push('\n');
    }
    let mut body = String::new();
    let mut guard_depth = 0;
    for line in implementation[start..].lines() {
        let trimmed = line.trim_start();
        if guard_depth > 0 {
            if trimmed.starts_with("#if") {
                guard_depth += 1;
            }
            if trimmed.starts_with("#endif") {
                guard_depth -= 1;
            }
        } else if trimmed.starts_with("#ifndef CXXBRIDGE1_STRUCT_")
            || trimmed.starts_with("#ifndef CXXBRIDGE1_ENUM_")
        {
            guard_depth = 1;
        } else if !forward.contains(line.trim()) {
            body.push_str(line);
            body.push('\n');
        }
    }
    if guard_depth != 0 {
        return Err("unclosed CXX shared-type guard".into());
    }
    let (rust, cxx) = partition(&body, false)?;
    Ok(Bridge {
        declarations: header,
        rust_api: Source {
            preamble: preamble.clone(),
            helpers: helpers.clone(),
            definitions: rust,
        },
        cxx_wrappers: Source {
            preamble,
            helpers,
            definitions: cxx,
        },
    })
}

// Skip literals and comments before looking for C++ statement delimiters.
// This parser is for generated CXX output, not arbitrary application C++.
fn skip_token(text: &str, pos: usize) -> Result<Option<usize>> {
    let bytes = text.as_bytes();
    if text[pos..].starts_with("//") {
        return Ok(Some(text[pos..].find('\n').map_or(text.len(), |n| pos + n)));
    }
    if text[pos..].starts_with("/*") {
        return text[pos + 2..]
            .find("*/")
            .map(|n| Some(pos + n + 4))
            .ok_or_else(|| "unclosed CXX comment".into());
    }
    if text[pos..].starts_with("R\"") {
        let open = text[pos + 2..].find('(').ok_or("invalid CXX raw string")? + pos + 2;
        let end = format!("){}\"", &text[pos + 2..open]);
        return text[open + 1..]
            .find(&end)
            .map(|n| Some(open + 1 + n + end.len()))
            .ok_or_else(|| "unclosed CXX raw string".into());
    }
    if matches!(bytes[pos], b'"' | b'\'') {
        let quote = bytes[pos];
        let mut i = pos + 1;
        while i < bytes.len() {
            if bytes[i] == b'\\' {
                i += 2;
            } else if bytes[i] == quote {
                return Ok(Some(i + 1));
            } else {
                i += 1;
            }
        }
        return Err("unclosed CXX string or character literal".into());
    }
    Ok(None)
}

fn trivia(text: &str, mut pos: usize) -> Result<usize> {
    while pos < text.len() {
        if text.as_bytes()[pos].is_ascii_whitespace() {
            pos += 1;
        } else if text[pos..].starts_with("//") || text[pos..].starts_with("/*") {
            pos = skip_token(text, pos)?.ok_or("invalid CXX comment")?;
        } else {
            break;
        }
    }
    Ok(pos)
}

fn close_brace(text: &str, open: usize) -> Result<usize> {
    let mut depth = 1;
    let mut i = open + 1;
    while i < text.len() {
        if !text.is_char_boundary(i) {
            i += 1;
            continue;
        }
        if let Some(end) = skip_token(text, i)? {
            i = end;
            continue;
        }
        match text.as_bytes()[i] {
            b'{' => depth += 1,
            b'}' => {
                depth -= 1;
                if depth == 0 {
                    return Ok(i);
                }
            }
            _ => {}
        }
        i += 1;
    }
    Err("unclosed CXX definition".into())
}

fn partition(text: &str, c_linkage: bool) -> Result<(String, String)> {
    let mut rust = String::new();
    let mut cxx = String::new();
    let mut pos = 0;
    while pos < text.len() {
        let start = trivia(text, pos)?;
        if start == text.len() {
            break;
        }
        if text.as_bytes()[start] == b'#' {
            let end = text[start..]
                .find('\n')
                .map_or(text.len(), |n| start + n + 1);
            let directive = &text[start..end];
            if !directive.starts_with("#pragma") {
                return Err(format!(
                    "unexpected directive in CXX definitions: {directive}"
                ));
            }
            rust.push_str(&text[pos..end]);
            cxx.push_str(&text[pos..end]);
            pos = end;
            continue;
        }
        let mut i = start;
        let mut delimiters = Vec::new();
        let (end, block) = loop {
            if i == text.len() {
                return Err("unterminated CXX statement".into());
            }
            if !text.is_char_boundary(i) {
                i += 1;
                continue;
            }
            if let Some(end) = skip_token(text, i)? {
                i = end;
                continue;
            }
            match text.as_bytes()[i] {
                b'(' => delimiters.push(b')'),
                b'[' => delimiters.push(b']'),
                b')' | b']' => {
                    if delimiters.pop() != Some(text.as_bytes()[i]) {
                        return Err("unbalanced CXX declaration".into());
                    }
                }
                b';' if delimiters.is_empty() => break (i + 1, None),
                b'{' if delimiters.is_empty() => {
                    let close = close_brace(text, i)?;
                    let mut end =
                        close + 1 + usize::from(text.as_bytes().get(close + 1) == Some(&b';'));
                    let mut comment = end;
                    while matches!(text.as_bytes().get(comment), Some(b' ' | b'\t')) {
                        comment += 1;
                    }
                    if text[comment..].starts_with("//") {
                        end = text[comment..]
                            .find('\n')
                            .map_or(text.len(), |n| comment + n);
                    }
                    break (end, Some((i, close)));
                }
                _ => {}
            }
            i += 1;
        };
        if let Some((open, close)) = block {
            let head = text[start..open].trim();
            if head.starts_with("namespace ")
                || head.starts_with("inline namespace ")
                || head == "extern \"C\""
                || head == "extern \"C++\""
            {
                let linkage = if head == "extern \"C\"" {
                    true
                } else if head == "extern \"C++\"" {
                    false
                } else {
                    c_linkage
                };
                let (r, c) = partition(&text[open + 1..close], linkage)?;
                for (out, body) in [(&mut rust, r), (&mut cxx, c)] {
                    if !body.trim().is_empty() {
                        out.push_str(&text[pos..open + 1]);
                        out.push_str(&body);
                        out.push_str(&text[close..end]);
                        out.push('\n');
                    }
                }
            } else {
                let out = if c_linkage || head.starts_with("extern \"C\"") {
                    &mut cxx
                } else {
                    &mut rust
                };
                out.push_str(&text[pos..end]);
                out.push('\n');
            }
        } else if text[start..end].starts_with("static_assert(") {
            cxx.push_str(&text[pos..end]);
            cxx.push('\n');
        } else if c_linkage || text[start..end].starts_with("extern \"C\"") {
            for out in [&mut rust, &mut cxx] {
                out.push_str(&text[pos..end]);
                out.push('\n');
            }
        } else {
            return Err(format!("unexpected CXX declaration: {}", &text[start..end]));
        }
        pos = end;
    }
    Ok((rust, cxx))
}
