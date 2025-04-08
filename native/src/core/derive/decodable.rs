use proc_macro2::TokenStream;
use quote::{quote, quote_spanned};
use syn::spanned::Spanned;
use syn::{Data, DeriveInput, Fields, GenericParam, parse_macro_input, parse_quote};

pub(crate) fn derive_decodable(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    let input = parse_macro_input!(input as DeriveInput);

    let name = input.ident;

    // Add a bound `T: Decodable` to every type parameter T.
    let mut generics = input.generics;
    for param in &mut generics.params {
        if let GenericParam::Type(ref mut type_param) = *param {
            type_param
                .bounds
                .push(parse_quote!(crate::socket::Decodable));
        }
    }

    let (impl_generics, ty_generics, where_clause) = generics.split_for_impl();

    let sum = gen_size_sum(&input.data);
    let encode = gen_encode(&input.data);
    let decode = gen_decode(&input.data);

    let expanded = quote! {
        // The generated impl.
        impl #impl_generics crate::socket::Encodable for #name #ty_generics #where_clause {
            fn encoded_len(&self) -> usize {
                #sum
            }
            fn encode(&self, w: &mut impl std::io::Write) -> std::io::Result<()> {
                #encode
                Ok(())
            }
        }
        impl #impl_generics crate::socket::Decodable for #name #ty_generics #where_clause {
            fn decode(r: &mut impl std::io::Read) -> std::io::Result<Self> {
                let val = #decode;
                Ok(val)
            }
        }
    };
    proc_macro::TokenStream::from(expanded)
}

// Generate an expression to sum up the size of each field.
fn gen_size_sum(data: &Data) -> TokenStream {
    match *data {
        Data::Struct(ref data) => {
            match data.fields {
                Fields::Named(ref fields) => {
                    // Expands to an expression like
                    //
                    //     0 + self.x.encoded_len() + self.y.encoded_len() + self.z.encoded_len()
                    let recurse = fields.named.iter().map(|f| {
                        let name = &f.ident;
                        quote_spanned! { f.span() =>
                            crate::socket::Encodable::encoded_len(&self.#name)
                        }
                    });
                    quote! {
                        0 #(+ #recurse)*
                    }
                }
                _ => unimplemented!(),
            }
        }
        Data::Enum(_) | Data::Union(_) => unimplemented!(),
    }
}

// Generate an expression to encode each field.
fn gen_encode(data: &Data) -> TokenStream {
    match *data {
        Data::Struct(ref data) => {
            match data.fields {
                Fields::Named(ref fields) => {
                    // Expands to an expression like
                    //
                    //     self.x.encode(w)?; self.y.encode(w)?; self.z.encode(w)?;
                    let recurse = fields.named.iter().map(|f| {
                        let name = &f.ident;
                        quote_spanned! { f.span() =>
                            crate::socket::Encodable::encode(&self.#name, w)?;
                        }
                    });
                    quote! {
                        #(#recurse)*
                    }
                }
                _ => unimplemented!(),
            }
        }
        Data::Enum(_) | Data::Union(_) => unimplemented!(),
    }
}

// Generate an expression to decode each field.
fn gen_decode(data: &Data) -> TokenStream {
    match *data {
        Data::Struct(ref data) => {
            match data.fields {
                Fields::Named(ref fields) => {
                    // Expands to an expression like
                    //
                    //     Self { x: Decodable::decode(r)?, y: Decodable::decode(r)?, }
                    let recurse = fields.named.iter().map(|f| {
                        let name = &f.ident;
                        quote_spanned! { f.span() =>
                            #name: crate::socket::Decodable::decode(r)?,
                        }
                    });
                    quote! {
                        Self { #(#recurse)* }
                    }
                }
                _ => unimplemented!(),
            }
        }
        Data::Enum(_) | Data::Union(_) => unimplemented!(),
    }
}
