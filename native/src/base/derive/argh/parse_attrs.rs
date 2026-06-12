// Copyright (c) 2020 Google LLC All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

use syn::parse::Parser;
use syn::punctuated::Punctuated;

use super::errors::Errors;
use proc_macro2::Span;
use std::collections::hash_map::{Entry, HashMap};

/// Attributes applied to a field of a `#![derive(FromArgs)]` struct.
#[derive(Default)]
pub struct FieldAttrs {
    pub default: Option<syn::LitStr>,
    pub description: Option<Description>,
    pub from_str_fn: Option<syn::ExprPath>,
    pub field_type: Option<FieldType>,
    pub long: Option<Option<syn::LitStr>>,
    pub short: Option<syn::LitChar>,
    pub arg_name: Option<syn::LitStr>,
    pub greedy: Option<syn::Path>,
    pub hidden_help: bool,
}

/// The purpose of a particular field on a `#![derive(FromArgs)]` struct.
#[derive(Copy, Clone, Eq, PartialEq)]
pub enum FieldKind {
    /// Switches are booleans that are set to "true" by passing the flag.
    Switch,
    /// Options are `--key value`. They may be optional (using `Option`),
    /// or repeating (using `Vec`), or required (neither `Option` nor `Vec`)
    Option,
    /// Subcommand fields (of which there can be at most one) refer to enums
    /// containing one of several potential subcommands. They may be optional
    /// (using `Option`) or required (no `Option`).
    SubCommand,
    /// Positional arguments are parsed literally if the input
    /// does not begin with `-` or `--` and is not a subcommand.
    /// They are parsed in declaration order, and only the last positional
    /// argument in a type may be an `Option`, `Vec`, or have a default value.
    Positional,
}

/// The type of a field on a `#![derive(FromArgs)]` struct.
///
/// This is a simple wrapper around `FieldKind` which includes the `syn::Ident`
/// of the attribute containing the field kind.
pub struct FieldType {
    pub kind: FieldKind,
    pub ident: syn::Ident,
}

/// A description of a `#![derive(FromArgs)]` struct.
///
/// Defaults to the docstring if one is present, or `#[argh(description = "...")]`
/// if one is provided.
pub struct Description {
    /// Whether the description was an explicit annotation or whether it was a doc string.
    pub explicit: bool,
    pub content: syn::LitStr,
}

impl FieldAttrs {
    pub fn parse(errors: &Errors, field: &syn::Field) -> Self {
        let mut this = Self::default();

        for attr in &field.attrs {
            if is_doc_attr(attr) {
                parse_attr_doc(errors, attr, &mut this.description);
                continue;
            }

            let ml = if let Some(ml) = argh_attr_to_meta_list(errors, attr) {
                ml
            } else {
                continue;
            };

            for meta in ml {
                let name = meta.path();
                if name.is_ident("arg_name") {
                    if let Some(m) = errors.expect_meta_name_value(&meta) {
                        this.parse_attr_arg_name(errors, m);
                    }
                } else if name.is_ident("default") {
                    if let Some(m) = errors.expect_meta_name_value(&meta) {
                        this.parse_attr_default(errors, m);
                    }
                } else if name.is_ident("description") {
                    if let Some(m) = errors.expect_meta_name_value(&meta) {
                        parse_attr_description(errors, m, &mut this.description);
                    }
                } else if name.is_ident("from_str_fn") {
                    if let Some(m) = errors.expect_meta_list(&meta) {
                        this.parse_attr_from_str_fn(errors, m);
                    }
                } else if name.is_ident("long") {
                    if let Some(m) = errors.expect_meta_name_value(&meta) {
                        this.parse_attr_long(errors, m);
                    }
                } else if name.is_ident("option") {
                    parse_attr_field_type(errors, &meta, FieldKind::Option, &mut this.field_type);
                } else if name.is_ident("short") {
                    if let Some(m) = errors.expect_meta_name_value(&meta) {
                        this.parse_attr_short(errors, m);
                    }
                } else if name.is_ident("subcommand") {
                    parse_attr_field_type(
                        errors,
                        &meta,
                        FieldKind::SubCommand,
                        &mut this.field_type,
                    );
                } else if name.is_ident("switch") {
                    parse_attr_field_type(errors, &meta, FieldKind::Switch, &mut this.field_type);
                } else if name.is_ident("positional") {
                    parse_attr_field_type(
                        errors,
                        &meta,
                        FieldKind::Positional,
                        &mut this.field_type,
                    );
                } else if name.is_ident("greedy") {
                    this.greedy = Some(name.clone());
                } else if name.is_ident("hidden_help") {
                    this.hidden_help = true;
                } else {
                    errors.err(
                        &meta,
                        concat!(
                            "Invalid field-level `argh` attribute\n",
                            "Expected one of: `arg_name`, `default`, `description`, `from_str_fn`, `greedy`, ",
                            "`long`, `option`, `short`, `subcommand`, `switch`, `hidden_help`",
                        ),
                    );
                }
            }
        }

        if let (Some(default), Some(field_type)) = (&this.default, &this.field_type) {
            match field_type.kind {
                FieldKind::Option | FieldKind::Positional => {}
                FieldKind::SubCommand | FieldKind::Switch => errors.err(
                    default,
                    "`default` may only be specified on `#[argh(option)]` \
                     or `#[argh(positional)]` fields",
                ),
            }
        }

        match (&this.greedy, this.field_type.as_ref().map(|f| f.kind)) {
            (Some(_), Some(FieldKind::Positional)) => {}
            (Some(greedy), Some(_)) => errors.err(
                &greedy,
                "`greedy` may only be specified on `#[argh(positional)]` \
                    fields",
            ),
            _ => {}
        }

        if let Some(d) = &this.description {
            check_option_description(errors, d.content.value().trim(), d.content.span());
        }

        this
    }

    fn parse_attr_from_str_fn(&mut self, errors: &Errors, m: &syn::MetaList) {
        parse_attr_fn_name(errors, m, "from_str_fn", &mut self.from_str_fn)
    }

    fn parse_attr_default(&mut self, errors: &Errors, m: &syn::MetaNameValue) {
        parse_attr_single_string(errors, m, "default", &mut self.default);
    }

    fn parse_attr_arg_name(&mut self, errors: &Errors, m: &syn::MetaNameValue) {
        parse_attr_single_string(errors, m, "arg_name", &mut self.arg_name);
    }

    fn parse_attr_long(&mut self, errors: &Errors, m: &syn::MetaNameValue) {
        if let Some(first) = &self.long {
            errors.duplicate_attrs("long", first, m);
        } else if let syn::Expr::Path(syn::ExprPath { path, .. }) = &m.value
            && let Some(ident) = path.get_ident()
            && ident.to_string().eq_ignore_ascii_case("none")
        {
            self.long = Some(None);
        } else if let Some(lit_str) = errors.expect_lit_str(&m.value) {
            self.long = Some(Some(lit_str.clone()));
        }
        if let Some(Some(long)) = &self.long {
            let value = long.value();
            check_long_name(errors, long, &value);
        }
    }

    fn parse_attr_short(&mut self, errors: &Errors, m: &syn::MetaNameValue) {
        if let Some(first) = &self.short {
            errors.duplicate_attrs("short", first, m);
        } else if let Some(lit_char) = errors.expect_lit_char(&m.value) {
            self.short = Some(lit_char.clone());
            if !lit_char.value().is_ascii() {
                errors.err(lit_char, "Short names must be ASCII");
            }
        }
    }
}

pub(crate) fn check_long_name(errors: &Errors, spanned: &impl syn::spanned::Spanned, value: &str) {
    if !value.is_ascii() {
        errors.err(spanned, "Long names must be ASCII");
    }
    if !value
        .chars()
        .all(|c| c.is_lowercase() || c == '-' || c.is_ascii_digit())
    {
        errors.err(
            spanned,
            "Long names may only contain lowercase letters, digits, and dashes",
        );
    }
}

fn parse_attr_fn_name(
    errors: &Errors,
    m: &syn::MetaList,
    attr_name: &str,
    slot: &mut Option<syn::ExprPath>,
) {
    if let Some(first) = slot {
        errors.duplicate_attrs(attr_name, first, m);
    }

    *slot = errors.ok(m.parse_args());
}

fn parse_attr_field_type(
    errors: &Errors,
    meta: &syn::Meta,
    kind: FieldKind,
    slot: &mut Option<FieldType>,
) {
    if let Some(path) = errors.expect_meta_word(meta) {
        if let Some(first) = slot {
            errors.duplicate_attrs("field kind", &first.ident, path);
        } else if let Some(word) = path.get_ident() {
            *slot = Some(FieldType {
                kind,
                ident: word.clone(),
            });
        }
    }
}

// Whether the attribute is one like `#[<name> ...]`
fn is_matching_attr(name: &str, attr: &syn::Attribute) -> bool {
    attr.path().segments.len() == 1 && attr.path().segments[0].ident == name
}

/// Checks for `#[doc ...]`, which is generated by doc comments.
fn is_doc_attr(attr: &syn::Attribute) -> bool {
    is_matching_attr("doc", attr)
}

/// Checks for `#[argh ...]`
fn is_argh_attr(attr: &syn::Attribute) -> bool {
    is_matching_attr("argh", attr)
}

/// Filters out non-`#[argh(...)]` attributes and converts to a sequence of `syn::Meta`.
fn argh_attr_to_meta_list(
    errors: &Errors,
    attr: &syn::Attribute,
) -> Option<impl IntoIterator<Item = syn::Meta>> {
    if !is_argh_attr(attr) {
        return None;
    }
    let ml = errors.expect_meta_list(&attr.meta)?;
    errors.ok(ml.parse_args_with(
        syn::punctuated::Punctuated::<syn::Meta, syn::Token![,]>::parse_terminated,
    ))
}

/// Represents a `#[derive(FromArgs)]` type's top-level attributes.
#[derive(Default)]
pub struct TypeAttrs {
    pub is_subcommand: Option<syn::Ident>,
    pub name: Option<syn::LitStr>,
    pub description: Option<Description>,
    pub examples: Vec<syn::LitStr>,
    pub notes: Vec<syn::LitStr>,
    pub error_codes: Vec<(syn::LitInt, syn::LitStr)>,
    /// Arguments that trigger printing of the help message
    pub help_triggers: Option<Vec<syn::LitStr>>,
}

impl TypeAttrs {
    /// Parse top-level `#[argh(...)]` attributes
    pub fn parse(errors: &Errors, derive_input: &syn::DeriveInput) -> Self {
        let mut this = TypeAttrs::default();

        for attr in &derive_input.attrs {
            if is_doc_attr(attr) {
                parse_attr_doc(errors, attr, &mut this.description);
                continue;
            }

            let ml: Vec<syn::Meta> = if let Some(ml) = argh_attr_to_meta_list(errors, attr) {
                ml.into_iter().collect()
            } else {
                continue;
            };

            for meta in ml.iter() {
                let name = meta.path();
                if name.is_ident("description") {
                    if let Some(m) = errors.expect_meta_name_value(meta) {
                        parse_attr_description(errors, m, &mut this.description);
                    }
                } else if name.is_ident("error_code") {
                    if let Some(m) = errors.expect_meta_list(meta) {
                        this.parse_attr_error_code(errors, m);
                    }
                } else if name.is_ident("example") {
                    if let Some(m) = errors.expect_meta_name_value(meta) {
                        this.parse_attr_example(errors, m);
                    }
                } else if name.is_ident("name") {
                    if let Some(m) = errors.expect_meta_name_value(meta) {
                        this.parse_attr_name(errors, m);
                    }
                } else if name.is_ident("note") {
                    if let Some(m) = errors.expect_meta_name_value(meta) {
                        this.parse_attr_note(errors, m);
                    }
                } else if name.is_ident("subcommand") {
                    if let Some(ident) = errors.expect_meta_word(meta).and_then(|p| p.get_ident()) {
                        this.parse_attr_subcommand(errors, ident);
                    }
                } else if name.is_ident("help_triggers") {
                    if let Some(m) = errors.expect_meta_list(meta) {
                        Self::parse_help_triggers(m, errors, &mut this);
                    }
                } else {
                    errors.err(
                        meta,
                        concat!(
                            "Invalid type-level `argh` attribute\n",
                            "Expected one of: `description`, `error_code`, `example`, `name`, ",
                            "`note`, `subcommand`, `help_triggers`",
                        ),
                    );
                }
            }

            if this.is_subcommand.is_some() && this.help_triggers.is_some() {
                let help_meta = ml
                    .iter()
                    .find(|meta| meta.path().is_ident("help_triggers"))
                    .unwrap();
                errors.err(help_meta, "Cannot use `help_triggers` on a subcommand");
            }
        }

        this.check_error_codes(errors);
        this
    }

    /// Checks that error codes are within range for `i32` and that they are
    /// never duplicated.
    fn check_error_codes(&self, errors: &Errors) {
        // map from error code to index
        let mut map: HashMap<u64, usize> = HashMap::new();
        for (index, (lit_int, _lit_str)) in self.error_codes.iter().enumerate() {
            let value = match lit_int.base10_parse::<u64>() {
                Ok(v) => v,
                Err(e) => {
                    errors.push(e);
                    continue;
                }
            };
            if value > (i32::MAX as u64) {
                errors.err(lit_int, "Error code out of range for `i32`");
            }
            match map.entry(value) {
                Entry::Occupied(previous) => {
                    let previous_index = *previous.get();
                    let (previous_lit_int, _previous_lit_str) = &self.error_codes[previous_index];
                    errors.err(lit_int, &format!("Duplicate error code {}", value));
                    errors.err(
                        previous_lit_int,
                        &format!("Error code {} previously defined here", value),
                    );
                }
                Entry::Vacant(slot) => {
                    slot.insert(index);
                }
            }
        }
    }

    fn parse_attr_error_code(&mut self, errors: &Errors, ml: &syn::MetaList) {
        errors.ok(ml.parse_args_with(|input: syn::parse::ParseStream| {
            let err_code = input.parse()?;
            input.parse::<syn::Token![,]>()?;
            let err_msg = input.parse()?;
            if let (Some(err_code), Some(err_msg)) = (
                errors.expect_lit_int(&err_code),
                errors.expect_lit_str(&err_msg),
            ) {
                self.error_codes.push((err_code.clone(), err_msg.clone()));
            }
            Ok(())
        }));
    }

    fn parse_attr_example(&mut self, errors: &Errors, m: &syn::MetaNameValue) {
        parse_attr_multi_string(errors, m, &mut self.examples)
    }

    fn parse_attr_name(&mut self, errors: &Errors, m: &syn::MetaNameValue) {
        parse_attr_single_string(errors, m, "name", &mut self.name);
        if let Some(name) = &self.name
            && name.value() == "help"
        {
            errors.err(name, "Custom `help` commands are not supported.");
        }
    }

    fn parse_attr_note(&mut self, errors: &Errors, m: &syn::MetaNameValue) {
        parse_attr_multi_string(errors, m, &mut self.notes)
    }

    fn parse_attr_subcommand(&mut self, errors: &Errors, ident: &syn::Ident) {
        if let Some(first) = &self.is_subcommand {
            errors.duplicate_attrs("subcommand", first, ident);
        } else {
            self.is_subcommand = Some(ident.clone());
        }
    }

    // get the list of arguments that trigger printing of the help message as a vector of strings (help_arguments("-h", "--help", "help"))
    fn parse_help_triggers(m: &syn::MetaList, errors: &Errors, this: &mut TypeAttrs) {
        let parser = Punctuated::<syn::Expr, syn::Token![,]>::parse_terminated;
        match parser.parse(m.tokens.clone().into()) {
            Ok(args) => {
                let mut triggers = Vec::new();
                for arg in args {
                    if let syn::Expr::Lit(syn::ExprLit {
                        lit: syn::Lit::Str(lit_str),
                        ..
                    }) = arg
                    {
                        triggers.push(lit_str);
                    }
                }

                this.help_triggers = Some(triggers);
            }
            Err(err) => errors.push(err),
        }
    }
}

/// Represents an enum variant's attributes.
#[derive(Default)]
pub struct VariantAttrs {
    pub is_dynamic: Option<syn::Path>,
}

impl VariantAttrs {
    /// Parse enum variant `#[argh(...)]` attributes
    pub fn parse(errors: &Errors, variant: &syn::Variant) -> Self {
        let mut this = VariantAttrs::default();

        let fields = match &variant.fields {
            syn::Fields::Named(fields) => Some(&fields.named),
            syn::Fields::Unnamed(fields) => Some(&fields.unnamed),
            syn::Fields::Unit => None,
        };

        for field in fields.into_iter().flatten() {
            for attr in &field.attrs {
                if is_argh_attr(attr) {
                    err_unused_enum_attr(errors, attr);
                }
            }
        }

        for attr in &variant.attrs {
            let ml = if let Some(ml) = argh_attr_to_meta_list(errors, attr) {
                ml
            } else {
                continue;
            };

            for meta in ml {
                let name = meta.path();
                if name.is_ident("dynamic") {
                    if let Some(prev) = this.is_dynamic.as_ref() {
                        errors.duplicate_attrs("dynamic", prev, &meta);
                    } else {
                        this.is_dynamic = errors.expect_meta_word(&meta).cloned();
                    }
                } else {
                    errors.err(
                        &meta,
                        "Invalid variant-level `argh` attribute\n\
                         Variants can only have the #[argh(dynamic)] attribute.",
                    );
                }
            }
        }

        this
    }
}

fn check_option_description(errors: &Errors, desc: &str, span: Span) {
    let chars = &mut desc.trim().chars();
    match (chars.next(), chars.next()) {
        (Some(x), _) if x.is_lowercase() => {}
        // If both the first and second letter are not lowercase,
        // this is likely an initialism which should be allowed.
        (Some(x), Some(y)) if !x.is_lowercase() && (y.is_alphanumeric() && !y.is_lowercase()) => {}
        _ => {
            errors.err_span(span, "Descriptions must begin with a lowercase letter");
        }
    }
}

fn parse_attr_single_string(
    errors: &Errors,
    m: &syn::MetaNameValue,
    name: &str,
    slot: &mut Option<syn::LitStr>,
) {
    if let Some(first) = slot {
        errors.duplicate_attrs(name, first, m);
    } else if let Some(lit_str) = errors.expect_lit_str(&m.value) {
        *slot = Some(lit_str.clone());
    }
}

fn parse_attr_multi_string(errors: &Errors, m: &syn::MetaNameValue, list: &mut Vec<syn::LitStr>) {
    if let Some(lit_str) = errors.expect_lit_str(&m.value) {
        list.push(lit_str.clone());
    }
}

fn parse_attr_doc(errors: &Errors, attr: &syn::Attribute, slot: &mut Option<Description>) {
    let nv = if let Some(nv) = errors.expect_meta_name_value(&attr.meta) {
        nv
    } else {
        return;
    };

    // Don't replace an existing explicit description.
    if slot.as_ref().map(|d| d.explicit).unwrap_or(false) {
        return;
    }

    if let Some(lit_str) = errors.expect_lit_str(&nv.value) {
        let lit_str = if let Some(previous) = slot {
            let previous = &previous.content;
            let previous_span = previous.span();
            syn::LitStr::new(
                &(previous.value() + &unescape_doc(lit_str.value())),
                previous_span,
            )
        } else {
            syn::LitStr::new(&unescape_doc(lit_str.value()), lit_str.span())
        };
        *slot = Some(Description {
            explicit: false,
            content: lit_str,
        });
    }
}

/// Replaces escape sequences in doc-comments with the characters they represent.
///
/// Rustdoc understands CommonMark escape sequences consisting of a backslash followed by an ASCII
/// punctuation character. Any other backslash is treated as a literal backslash.
fn unescape_doc(s: String) -> String {
    let mut result = String::with_capacity(s.len());

    let mut characters = s.chars().peekable();
    while let Some(mut character) = characters.next() {
        if character == '\\'
            && let Some(next_character) = characters.peek()
            && next_character.is_ascii_punctuation()
        {
            character = *next_character;
            characters.next();
        }

        // Braces must be escaped as this string will be used as a format string
        if character == '{' || character == '}' {
            result.push(character);
        }

        result.push(character);
    }

    result
}

fn parse_attr_description(errors: &Errors, m: &syn::MetaNameValue, slot: &mut Option<Description>) {
    let lit_str = if let Some(lit_str) = errors.expect_lit_str(&m.value) {
        lit_str
    } else {
        return;
    };

    // Don't allow multiple explicit (non doc-comment) descriptions
    if let Some(description) = slot
        && description.explicit
    {
        errors.duplicate_attrs("description", &description.content, lit_str);
    }

    *slot = Some(Description {
        explicit: true,
        content: lit_str.clone(),
    });
}

/// Checks that a `#![derive(FromArgs)]` enum has an `#[argh(subcommand)]`
/// attribute and that it does not have any other type-level `#[argh(...)]` attributes.
pub fn check_enum_type_attrs(errors: &Errors, type_attrs: &TypeAttrs, type_span: &Span) {
    let TypeAttrs {
        is_subcommand,
        name,
        description,
        examples,
        notes,
        error_codes,
        help_triggers,
    } = type_attrs;

    // Ensure that `#[argh(subcommand)]` is present.
    if is_subcommand.is_none() {
        errors.err_span(
            *type_span,
            concat!(
                "`#![derive(FromArgs)]` on `enum`s can only be used to enumerate subcommands.\n",
                "Consider adding `#[argh(subcommand)]` to the `enum` declaration.",
            ),
        );
    }

    // Error on all other type-level attributes.
    if let Some(name) = name {
        err_unused_enum_attr(errors, name);
    }
    if let Some(description) = description
        && description.explicit
    {
        err_unused_enum_attr(errors, &description.content);
    }
    if let Some(example) = examples.first() {
        err_unused_enum_attr(errors, example);
    }
    if let Some(note) = notes.first() {
        err_unused_enum_attr(errors, note);
    }
    if let Some(err_code) = error_codes.first() {
        err_unused_enum_attr(errors, &err_code.0);
    }
    if let Some(triggers) = help_triggers
        && let Some(trigger) = triggers.first()
    {
        err_unused_enum_attr(errors, trigger);
    }
}

fn err_unused_enum_attr(errors: &Errors, location: &impl syn::spanned::Spanned) {
    errors.err(
        location,
        concat!(
            "Unused `argh` attribute on `#![derive(FromArgs)]` enum. ",
            "Such `enum`s can only be used to dispatch to subcommands, ",
            "and should only contain the #[argh(subcommand)] attribute.",
        ),
    );
}
