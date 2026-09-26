#![recursion_limit = "256"]

use proc_macro::TokenStream;

mod argh;
#[path = "../../include/bridge.rs"]
mod bridge;
mod decodable;

#[proc_macro_attribute]
pub fn bridge(args: TokenStream, input: TokenStream) -> TokenStream {
    let mut module = syn::parse_macro_input!(input as syn::ItemMod);
    if let Err(error) = bridge::prepare(&mut module) {
        return error.into_compile_error().into();
    }
    let args = proc_macro2::TokenStream::from(args);
    quote::quote!(#[cxx::bridge(#args)] #module).into()
}

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
