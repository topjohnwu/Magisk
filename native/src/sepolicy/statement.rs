use crate::ffi::Xperm;
use pomelo::pomelo;

pomelo! {
    %module statement;
    %parser pub struct Parser<'a>{};
    %extra_argument Pin<&'a mut sepolicy>;
    %token pub enum Token<'a> {};

    %include {
        use std::pin::Pin;
        use super::Xperm;
        use crate::sepolicy;
    }

    %type HX u16;
    %type ID &'a str;

    %type statement ();
    %type name &'a str;
    %type names Vec<&'a str>;
    %type sterm Vec<&'a str>;
    %type term Vec<&'a str>;
    %type xperms Vec<Xperm>;
    %type xperm_list Vec<Xperm>;
    %type xperm Xperm;

    statement ::= AL sterm(s) sterm(t) sterm(c) sterm(p) { extra.as_mut().allow(s, t, c, p); };
    statement ::= DN sterm(s) sterm(t) sterm(c) sterm(p) { extra.as_mut().deny(s, t, c, p); };
    statement ::= AA sterm(s) sterm(t) sterm(c) sterm(p) { extra.as_mut().auditallow(s, t, c, p); };
    statement ::= DA sterm(s) sterm(t) sterm(c) sterm(p) { extra.as_mut().dontaudit(s, t, c, p); };
    statement ::= AX sterm(s) sterm(t) sterm(c) IO xperms(p) { extra.as_mut().allowxperm(s, t, c, p); };
    statement ::= AY sterm(s) sterm(t) sterm(c) IO xperms(p) { extra.as_mut().auditallowxperm(s, t, c, p); };
    statement ::= DX sterm(s) sterm(t) sterm(c) IO xperms(p) { extra.as_mut().dontauditxperm(s, t, c, p); };
    statement ::= PM sterm(t) { extra.as_mut().permissive(t); };
    statement ::= EF sterm(t) { extra.as_mut().enforce(t); };
    statement ::= TA term(t) term(a) { extra.as_mut().typeattribute(t, a); };
    statement ::= TY ID(t) { extra.as_mut().type_(t, vec![]);};
    statement ::= TY ID(t) term(a) { extra.as_mut().type_(t, a);};
    statement ::= AT ID(t) { extra.as_mut().attribute(t); };
    statement ::= TT ID(s) ID(t) ID(c) ID(d) { extra.as_mut().type_transition(s, t, c, d, vec![]); };
    statement ::= TT ID(s) ID(t) ID(c) ID(d) ID(o) { extra.as_mut().type_transition(s, t, c, d, vec![o]); };
    statement ::= TC ID(s) ID(t) ID(c) ID(d) { extra.as_mut().type_change(s, t, c, d); };
    statement ::= TM ID(s) ID(t) ID(c) ID(d) { extra.as_mut().type_member(s, t, c, d);};
    statement ::= GF ID(s) ID(t) ID(c) { extra.as_mut().genfscon(s, t, c); };

    xperms ::= HX(low) { if low > 0 { vec![Xperm{low, high: 0, reset: false}] } else { vec![Xperm{low: 0x0000, high: 0xFFFF, reset: true}] }};
    xperms ::= LB xperm_list(l) RB { l };
    xperms ::= TL LB xperm_list(mut l) RB { l.iter_mut().for_each(|x| { x.reset = true; }); l };
    xperms ::= ST { vec![Xperm{low: 0x0000, high: 0xFFFF, reset: false}] };

    xperm_list ::= xperm(p) { vec![p] }
    xperm_list ::= xperm_list(mut l) xperm(p) { l.push(p); l }

    xperm ::= HX(low) { Xperm{low, high: 0, reset: false} };
    xperm ::= HX(low) HP HX(high) { Xperm{low, high, reset: false} };

    sterm ::= ST { vec![] }
    sterm ::= term(n) { n }
    term ::= ID(n) { vec![n] }
    term ::= LB names(n) RB { n };
    names ::= ID(n) { vec![n] };
    names ::= names(mut v) CM ID(n) { v.push(n); v };
}

pub use statement::{Parser, Token};

pub fn tokenize_statement<'a>(statement: &'a str) -> Vec<Token<'a>> {
    let mut tokens = Vec::new();
    for s in statement.trim().split_whitespace() {
        let mut res = Some(s);
        while let Some(s) = res {
            match s {
                "allow" => tokens.push(Token::AL),
                "deny" => tokens.push(Token::DN),
                "auditallow" => tokens.push(Token::AA),
                "dontaudit" => tokens.push(Token::DA),
                "allowxperm" => tokens.push(Token::AX),
                "auditallowxperm" => tokens.push(Token::AY),
                "dontauditxperm" => tokens.push(Token::DX),
                "permissive" => tokens.push(Token::PM),
                "enforce" => tokens.push(Token::EF),
                "typeattribute" => tokens.push(Token::TA),
                "type" => tokens.push(Token::TY),
                "attribute" => tokens.push(Token::AT),
                "type_transition" => tokens.push(Token::TT),
                "type_change" => tokens.push(Token::TC),
                "type_member" => tokens.push(Token::TM),
                "genfscon" => tokens.push(Token::GF),
                "ioctl" => tokens.push(Token::IO),
                "" => {}
                _ => {
                    if let Some(s) = s.strip_prefix("{") {
                        tokens.push(Token::LB);
                        res = Some(s);
                        continue;
                    } else if let Some(s) = s.strip_prefix("}") {
                        tokens.push(Token::RB);
                        res = Some(s);
                        continue;
                    } else if let Some(s) = s.strip_prefix(",") {
                        tokens.push(Token::CM);
                        res = Some(s);
                        continue;
                    } else if let Some(s) = s.strip_prefix("*") {
                        tokens.push(Token::ST);
                        res = Some(s);
                        continue;
                    } else if let Some(s) = s.strip_prefix("~") {
                        res = Some(s);
                        tokens.push(Token::TL);
                        continue;
                    } else if let Some(s) = s.strip_prefix("-") {
                        res = Some(s);
                        tokens.push(Token::HP);
                        continue;
                    } else if let Some(s) = s.strip_prefix("0x") {
                        tokens.push(Token::HX(s.parse().unwrap_or(0)));
                    } else {
                        tokens.push(Token::ID(s));
                    }
                }
            }
            break;
        }
    }
    tokens
}
