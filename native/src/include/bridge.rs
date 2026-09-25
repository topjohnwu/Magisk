// Shared by the Rust attribute macro and the C++ binding generator.

use syn::ext::IdentExt;
use syn::parse::{Parse, ParseStream, Parser};
use syn::{Error, ForeignItem, Ident, Item, ItemMod, LitBool, LitStr, Result, Token, parse_quote};

pub const IMPORT: &str = "__magisk_import__";
pub const EXPORT_IMPORT: &str = "__magisk_export_import__";

struct Import {
    module: LitStr,
    export: bool,
}

impl Parse for Import {
    fn parse(input: ParseStream) -> Result<Self> {
        let module: LitStr = input.parse()?;
        let name = module.value();
        let valid_name = |name: &str| {
            name.split('.').all(|part| {
                !part.contains('#') && Ident::parse_any.parse_str(part).is_ok_and(|id| id == part)
            })
        };
        let (name, partition) = name
            .split_once(':')
            .map_or((name.as_str(), None), |(n, p)| (n, Some(p)));
        if !(valid_name(name) || name.is_empty() && partition.is_some())
            || partition.is_some_and(|part| !valid_name(part))
        {
            return Err(Error::new(
                module.span(),
                "expected a C++ module name or partition",
            ));
        }
        let mut export = false;
        if input.parse::<Option<Token![,]>>()?.is_some() && !input.is_empty() {
            let key: Ident = input.parse()?;
            if key != "export" {
                return Err(Error::new(key.span(), "expected export"));
            }
            input.parse::<Token![=]>()?;
            export = input.parse::<LitBool>()?.value;
            input.parse::<Option<Token![,]>>()?;
        }
        Ok(Self { module, export })
    }
}

pub fn prepare(module: &mut ItemMod) -> Result<()> {
    let Some((_, items)) = &mut module.content else {
        return Err(Error::new_spanned(
            &module.ident,
            "bridge module must have inline contents",
        ));
    };
    let mut imports = Vec::new();
    for item in &mut *items {
        let Item::ForeignMod(block) = item else {
            continue;
        };
        let mut retained = Vec::new();
        for item in std::mem::take(&mut block.items) {
            if let ForeignItem::Macro(mac) = &item
                && mac.mac.path.is_ident("import")
            {
                if !block
                    .abi
                    .name
                    .as_ref()
                    .is_some_and(|name| matches!(name.value().as_str(), "C++" | "C++-unwind"))
                {
                    return Err(Error::new_spanned(
                        mac,
                        "import! requires an extern C++ block",
                    ));
                }
                let import: Import = mac.mac.parse_body()?;
                let prefix = if import.export { EXPORT_IMPORT } else { IMPORT };
                let marker = LitStr::new(
                    &format!("{prefix}{}", import.module.value()),
                    import.module.span(),
                );
                // CXX already transports includes to the generator and ignores
                // them during Rust expansion. Hoist item cfg to the extern block
                // so CXX evaluates both block and import conditions correctly.
                let mut dependency = block.clone();
                dependency.attrs.extend(mac.attrs.iter().cloned());
                dependency.items = vec![parse_quote!(include!(#marker);)];
                imports.push(Item::ForeignMod(dependency));
            } else {
                retained.push(item);
            }
        }
        block.items = retained;
    }
    items.extend(imports);
    Ok(())
}

#[cfg(not(proc_macro))]
pub fn source(source: &str) -> Result<proc_macro2::TokenStream> {
    fn visit(items: &mut [Item]) -> Result<()> {
        for item in items {
            let Item::Mod(module) = item else { continue };
            let bridge = module.attrs.iter_mut().find(|attr| {
                let path = attr
                    .path()
                    .segments
                    .iter()
                    .map(|s| s.ident.to_string())
                    .collect::<Vec<_>>();
                path == ["derive", "bridge"] || path == ["base", "derive", "bridge"]
            });
            if let Some(attr) = bridge {
                let path = match &mut attr.meta {
                    syn::Meta::Path(path) => path,
                    syn::Meta::List(meta) => &mut meta.path,
                    syn::Meta::NameValue(meta) => &mut meta.path,
                };
                *path = parse_quote!(cxx::bridge);
                prepare(module)?;
            } else if let Some((_, items)) = &mut module.content {
                visit(items)?;
            }
        }
        Ok(())
    }
    let mut file = syn::parse_file(source)?;
    visit(&mut file.items)?;
    Ok(quote::quote!(#file))
}
