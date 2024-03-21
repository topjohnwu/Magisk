use std::{iter::Peekable, pin::Pin, vec::IntoIter};

use base::{LoggedError, LoggedResult};

use crate::ffi::Xperm;
use crate::sepolicy;

pub enum Token<'a> {
    AL,
    DN,
    AA,
    DA,
    AX,
    AY,
    DX,
    PM,
    EF,
    TA,
    TY,
    AT,
    TT,
    TC,
    TM,
    GF,
    IO,
    LB,
    RB,
    CM,
    ST,
    TL,
    HP,
    HX(u16),
    ID(&'a str),
}

type Tokens<'a> = Peekable<IntoIter<Token<'a>>>;

macro_rules! throw {
    () => {
        Err(LoggedError::default())?
    };
}

fn parse_id<'a>(tokens: &mut Tokens<'a>) -> LoggedResult<&'a str> {
    match tokens.next() {
        Some(Token::ID(name)) => Ok(name),
        _ => throw!(),
    }
}

//     names ::= ID(n) { vec![n] };
//     names ::= names(mut v) CM ID(n) { v.push(n); v };
//     term ::= ID(n) { vec![n] }
//     term ::= LB names(n) RB { n };
fn parse_term<'a>(tokens: &mut Tokens<'a>) -> LoggedResult<Vec<&'a str>> {
    match tokens.next() {
        Some(Token::ID(name)) => Ok(vec![name]),
        Some(Token::LB) => {
            let mut names = Vec::new();
            loop {
                names.push(parse_id(tokens)?);
                match tokens.peek() {
                    Some(Token::CM) => {
                        tokens.next();
                    }
                    _ => break,
                }
            }
            if matches!(tokens.next(), Some(Token::RB)) {
                return Ok(names);
            }
            throw!()
        }
        _ => throw!(),
    }
}

//     terms ::= ST { vec![] }
//     terms ::= term(n) { n }
fn parse_terms<'a>(tokens: &mut Tokens<'a>) -> LoggedResult<Vec<&'a str>> {
    match tokens.peek() {
        Some(Token::ST) => {
            tokens.next();
            Ok(Vec::new())
        }
        _ => parse_term(tokens),
    }
}

//     xperm ::= HX(low) { Xperm{low, high: 0, reset: false} };
//     xperm ::= HX(low) HP HX(high) { Xperm{low, high, reset: false} };
fn parse_xperm(tokens: &mut Tokens) -> LoggedResult<Xperm> {
    let low = match tokens.next() {
        Some(Token::HX(low)) => low,
        _ => throw!(),
    };
    let high = match tokens.peek() {
        Some(Token::HP) => {
            tokens.next();
            match tokens.next() {
                Some(Token::HX(high)) => high,
                _ => throw!(),
            }
        }
        _ => 0,
    };
    Ok(Xperm {
        low,
        high,
        reset: false,
    })
}

//     xperms ::= HX(low) { if low > 0 { vec![Xperm{low, high: 0, reset: false}] } else { vec![Xperm{low: 0x0000, high: 0xFFFF, reset: true}] }};
//     xperms ::= LB xperm_list(l) RB { l };
//     xperms ::= TL LB xperm_list(mut l) RB { l.iter_mut().for_each(|x| { x.reset = true; }); l };
//     xperms ::= ST { vec![Xperm{low: 0x0000, high: 0xFFFF, reset: false}] };
//
//     xperm_list ::= xperm(p) { vec![p] }
//     xperm_list ::= xperm_list(mut l) xperm(p) { l.push(p); l }
fn parse_xperms(tokens: &mut Tokens) -> LoggedResult<Vec<Xperm>> {
    let mut xperms = Vec::new();
    let reset = match tokens.peek() {
        Some(Token::TL) => {
            tokens.next();
            if !matches!(tokens.peek(), Some(Token::LB)) {
                throw!();
            }
            true
        }
        _ => false,
    };
    match tokens.next() {
        Some(Token::LB) => {
            // parse xperm_list
            loop {
                let mut xperm = parse_xperm(tokens)?;
                xperm.reset = reset;
                xperms.push(xperm);
                if matches!(tokens.peek(), Some(Token::RB)) {
                    tokens.next();
                    break;
                }
            }
        }
        Some(Token::ST) => {
            xperms.push(Xperm {
                low: 0x0000,
                high: 0xFFFF,
                reset,
            });
        }
        Some(Token::HX(low)) => {
            if low > 0 {
                xperms.push(Xperm {
                    low,
                    high: 0,
                    reset,
                });
            } else {
                xperms.push(Xperm {
                    low: 0x0000,
                    high: 0xFFFF,
                    reset,
                });
            }
        }
        _ => throw!(),
    }
    Ok(xperms)
}

//     statement ::= AL sterm(s) sterm(t) sterm(c) sterm(p) { extra.as_mut().allow(s, t, c, p); };
//     statement ::= DN sterm(s) sterm(t) sterm(c) sterm(p) { extra.as_mut().deny(s, t, c, p); };
//     statement ::= AA sterm(s) sterm(t) sterm(c) sterm(p) { extra.as_mut().auditallow(s, t, c, p); };
//     statement ::= DA sterm(s) sterm(t) sterm(c) sterm(p) { extra.as_mut().dontaudit(s, t, c, p); };
//     statement ::= AX sterm(s) sterm(t) sterm(c) IO xperms(p) { extra.as_mut().allowxperm(s, t, c, p); };
//     statement ::= AY sterm(s) sterm(t) sterm(c) IO xperms(p) { extra.as_mut().auditallowxperm(s, t, c, p); };
//     statement ::= DX sterm(s) sterm(t) sterm(c) IO xperms(p) { extra.as_mut().dontauditxperm(s, t, c, p); };
//     statement ::= PM sterm(t) { extra.as_mut().permissive(t); };
//     statement ::= EF sterm(t) { extra.as_mut().enforce(t); };
//     statement ::= TA term(t) term(a) { extra.as_mut().typeattribute(t, a); };
//     statement ::= TY ID(t) { extra.as_mut().type_(t, vec![]);};
//     statement ::= TY ID(t) term(a) { extra.as_mut().type_(t, a);};
//     statement ::= AT ID(t) { extra.as_mut().attribute(t); };
//     statement ::= TT ID(s) ID(t) ID(c) ID(d) { extra.as_mut().type_transition(s, t, c, d, vec![]); };
//     statement ::= TT ID(s) ID(t) ID(c) ID(d) ID(o) { extra.as_mut().type_transition(s, t, c, d, vec![o]); };
//     statement ::= TC ID(s) ID(t) ID(c) ID(d) { extra.as_mut().type_change(s, t, c, d); };
//     statement ::= TM ID(s) ID(t) ID(c) ID(d) { extra.as_mut().type_member(s, t, c, d);};
//     statement ::= GF ID(s) ID(t) ID(c) { extra.as_mut().genfscon(s, t, c); };
fn exec_statement(sepolicy: Pin<&mut sepolicy>, tokens: &mut Tokens) -> LoggedResult<()> {
    let action = match tokens.next() {
        Some(token) => token,
        _ => throw!(),
    };
    match action {
        Token::AL | Token::DN | Token::AA | Token::DA => {
            let s = parse_terms(tokens)?;
            let t = parse_terms(tokens)?;
            let c = parse_terms(tokens)?;
            let p = parse_terms(tokens)?;
            match action {
                Token::AL => sepolicy.allow(s, t, c, p),
                Token::DN => sepolicy.deny(s, t, c, p),
                Token::AA => sepolicy.auditallow(s, t, c, p),
                Token::DA => sepolicy.dontaudit(s, t, c, p),
                _ => unreachable!(),
            }
        }
        Token::AX | Token::AY | Token::DX => {
            let s = parse_terms(tokens)?;
            let t = parse_terms(tokens)?;
            let c = parse_terms(tokens)?;
            let p = if matches!(tokens.next(), Some(Token::IO)) {
                parse_xperms(tokens)?
            } else {
                throw!()
            };
            match action {
                Token::AX => sepolicy.allowxperm(s, t, c, p),
                Token::AY => sepolicy.auditallowxperm(s, t, c, p),
                Token::DX => sepolicy.dontauditxperm(s, t, c, p),
                _ => unreachable!(),
            }
        }
        Token::PM | Token::EF => {
            let t = parse_terms(tokens)?;
            match action {
                Token::PM => sepolicy.permissive(t),
                Token::EF => sepolicy.enforce(t),
                _ => unreachable!(),
            }
        }
        Token::TA => {
            let t = parse_term(tokens)?;
            let a = parse_term(tokens)?;
            sepolicy.typeattribute(t, a)
        }
        Token::TY => {
            let t = parse_id(tokens)?;
            let a = if tokens.peek().is_none() {
                vec![]
            } else {
                parse_term(tokens)?
            };
            sepolicy.type_(t, a)
        }
        Token::AT => {
            let t = parse_id(tokens)?;
            sepolicy.attribute(t)
        }
        Token::TT | Token::TC | Token::TM => {
            let s = parse_id(tokens)?;
            let t = parse_id(tokens)?;
            let c = parse_id(tokens)?;
            let d = parse_id(tokens)?;
            match action {
                Token::TT => {
                    let o = if tokens.peek().is_none() {
                        vec![]
                    } else {
                        vec![parse_id(tokens)?]
                    };
                    sepolicy.type_transition(s, t, c, d, o)
                }
                Token::TC => sepolicy.type_change(s, t, c, d),
                Token::TM => sepolicy.type_member(s, t, c, d),
                _ => unreachable!(),
            }
        }
        Token::GF => {
            let s = parse_id(tokens)?;
            let t = parse_id(tokens)?;
            let c = parse_id(tokens)?;
            sepolicy.genfscon(s, t, c)
        }
        _ => throw!(),
    }
    if tokens.peek().is_none() {
        Ok(())
    } else {
        throw!()
    }
}

fn tokenize_statement(statement: &str) -> Vec<Token> {
    let mut tokens = Vec::new();
    for s in statement.split_whitespace() {
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
                    if let Some(s) = s.strip_prefix('{') {
                        tokens.push(Token::LB);
                        res = Some(s);
                        continue;
                    } else if let Some(s) = s.strip_prefix('}') {
                        tokens.push(Token::RB);
                        res = Some(s);
                        continue;
                    } else if let Some(s) = s.strip_prefix(',') {
                        tokens.push(Token::CM);
                        res = Some(s);
                        continue;
                    } else if let Some(s) = s.strip_prefix('*') {
                        tokens.push(Token::ST);
                        res = Some(s);
                        continue;
                    } else if let Some(s) = s.strip_prefix('~') {
                        res = Some(s);
                        tokens.push(Token::TL);
                        continue;
                    } else if let Some(s) = s.strip_prefix('-') {
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

pub fn parse_statement(sepolicy: Pin<&mut sepolicy>, statement: &str) -> LoggedResult<()> {
    let mut tokens = tokenize_statement(statement).into_iter().peekable();
    exec_statement(sepolicy, &mut tokens)
}
