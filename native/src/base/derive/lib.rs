#![recursion_limit = "256"]

use proc_macro::TokenStream;

mod argh;
mod decodable;

#[proc_macro_derive(Decodable)]
pub fn derive_decodable(input: TokenStream) -> TokenStream {
    decodable::derive_decodable(input)
}

/// Entrypoint for `#[derive(FromArgs)]`.
#[proc_macro_derive(FromArgs, attributes(argh))]
pub fn argh_derive(input: TokenStream) -> TokenStream {
    let ast = syn::parse_macro_input!(input as syn::DeriveInput);
    let token = argh::impl_from_args(&ast);
    token.into()
}
