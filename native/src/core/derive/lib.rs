use proc_macro::TokenStream;

mod decodable;

#[proc_macro_derive(Decodable)]
pub fn derive_decodable(input: TokenStream) -> TokenStream {
    decodable::derive_decodable(input)
}
