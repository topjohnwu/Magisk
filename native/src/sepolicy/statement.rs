use std::fmt::{Display, Formatter, Write};
use std::io::{BufRead, BufReader, Cursor};
use std::iter::Peekable;
use std::vec::IntoIter;

use crate::SePolicy;
use crate::ffi::Xperm;
use base::nix::fcntl::OFlag;
use base::{BufReadExt, LoggedResult, Utf8CStr, error, warn};

#[derive(Debug, PartialEq)]
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
    LB,
    RB,
    CM,
    ST,
    TL,
    HP,
    HX(u16),
    ID(&'a str),
}

#[derive(Debug, PartialEq)]
pub enum PolicyStatement<'a> {
    Allow {
        src: Vec<&'a str>,
        target: Vec<&'a str>,
        class: Vec<&'a str>,
        perm: Vec<&'a str>,
    },
    Deny {
        src: Vec<&'a str>,
        target: Vec<&'a str>,
        class: Vec<&'a str>,
        perm: Vec<&'a str>,
    },
    AuditAllow {
        src: Vec<&'a str>,
        target: Vec<&'a str>,
        class: Vec<&'a str>,
        perm: Vec<&'a str>,
    },
    DontAudit {
        src: Vec<&'a str>,
        target: Vec<&'a str>,
        class: Vec<&'a str>,
        perm: Vec<&'a str>,
    },
    AllowXperm {
        src: Vec<&'a str>,
        target: Vec<&'a str>,
        class: Vec<&'a str>,
        xperm: Vec<Xperm>,
    },
    AuditAllowXperm {
        src: Vec<&'a str>,
        target: Vec<&'a str>,
        class: Vec<&'a str>,
        xperm: Vec<Xperm>,
    },
    DontAuditXperm {
        src: Vec<&'a str>,
        target: Vec<&'a str>,
        class: Vec<&'a str>,
        xperm: Vec<Xperm>,
    },
    Permissive {
        type_: Vec<&'a str>,
    },
    Enforce {
        type_: Vec<&'a str>,
    },
    TypeAttribute {
        type_: Vec<&'a str>,
        attr: Vec<&'a str>,
    },
    Type {
        type_: &'a str,
        attr: Vec<&'a str>,
    },
    Attribute {
        attr: &'a str,
    },
    TypeTransition {
        src: &'a str,
        target: &'a str,
        class: &'a str,
        default: &'a str,
        object: &'a str,
    },
    TypeChange {
        src: &'a str,
        target: &'a str,
        class: &'a str,
        default: &'a str,
    },
    TypeMember {
        src: &'a str,
        target: &'a str,
        class: &'a str,
        default: &'a str,
    },
    GenfsCon {
        fs_name: &'a str,
        path: &'a str,
        context: &'a str,
    },
}

type Tokens<'a> = Peekable<IntoIter<Token<'a>>>;
type ParseResult<'a, T> = Result<T, ParseError<'a>>;

#[derive(Debug, PartialEq)]
enum ParseError<'a> {
    General,
    AvtabAv(Token<'a>),
    AvtabXperms(Token<'a>),
    AvtabType(Token<'a>),
    TypeState(Token<'a>),
    TypeAttr,
    TypeTrans,
    NewType,
    NewAttr,
    GenfsCon,
    ShowHelp,
    UnknownAction(Token<'a>),
}

macro_rules! throw {
    () => {
        Err(ParseError::General)?
    };
}

fn parse_id<'a>(tokens: &mut Tokens<'a>) -> ParseResult<'a, &'a str> {
    match tokens.next() {
        Some(Token::ID(name)) => Ok(name),
        _ => throw!(),
    }
}

// names ::= ID(n) { vec![n] };
// names ::= names(mut v) ID(n) { v.push(n); v };
// term ::= ID(n) { vec![n] }
// term ::= LB names(n) RB { n };
fn parse_term<'a>(tokens: &mut Tokens<'a>) -> ParseResult<'a, Vec<&'a str>> {
    match tokens.next() {
        Some(Token::ID(name)) => Ok(vec![name]),
        Some(Token::LB) => {
            let mut names = Vec::new();
            loop {
                match tokens.next() {
                    Some(Token::ID(name)) => names.push(name),
                    Some(Token::RB) => break,
                    _ => throw!(),
                }
            }
            Ok(names)
        }
        _ => throw!(),
    }
}

// names ::= ST { vec![] }
// names ::= ID(n) { vec![n] };
// names ::= names(mut v) ID(n) { v.push(n); v };
// names ::= names(n) ST { vec![] };
// sterm ::= ST { vec![] }
// sterm ::= ID(n) { vec![n] }
// sterm ::= LB names(n) RB { n };
fn parse_sterm<'a>(tokens: &mut Tokens<'a>) -> ParseResult<'a, Vec<&'a str>> {
    match tokens.next() {
        Some(Token::ID(name)) => Ok(vec![name]),
        Some(Token::ST) => Ok(vec![]),
        Some(Token::LB) => {
            let mut names = Some(Vec::new());
            loop {
                match tokens.next() {
                    Some(Token::ID(name)) => {
                        if let Some(ref mut names) = names {
                            names.push(name)
                        }
                    }
                    Some(Token::ST) => names = None,
                    Some(Token::RB) => break,
                    _ => throw!(),
                }
            }
            Ok(names.unwrap_or(vec![]))
        }
        _ => throw!(),
    }
}

fn parse_xperm_hex(s: &str) -> Option<u16> {
    s.strip_prefix("0x")
        .and_then(|s| u16::from_str_radix(s, 16).ok())
}

// xperm ::= HX(low) { Xperm{low, high: low, reset: false} };
// xperm ::= HX(low) HP HX(high) { Xperm{low, high, reset: false} };
fn parse_xperm<'a>(tokens: &mut Tokens<'a>) -> ParseResult<'a, Xperm> {
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
        _ => low,
    };
    Ok(Xperm {
        low,
        high,
        reset: false,
    })
}

// xperms ::= HX(low) { if low > 0 { vec![Xperm{low, high: low, reset: false}] } else { vec![Xperm{low: 0x0000, high: 0xFFFF, reset: true}] }};
// xperms ::= LB xperm_list(l) RB { l };
// xperms ::= TL LB xperm_list(mut l) RB { l.iter_mut().for_each(|x| { x.reset = true; }); l };
// xperms ::= ST { vec![Xperm{low: 0x0000, high: 0xFFFF, reset: false}] };
//
// xperm_list ::= xperm(p) { vec![p] }
// xperm_list ::= xperm_list(mut l) xperm(p) { l.push(p); l }
fn parse_xperms<'a>(tokens: &mut Tokens<'a>) -> ParseResult<'a, Vec<Xperm>> {
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
    match tokens.peek() {
        Some(Token::LB) => {
            tokens.next();
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
            tokens.next();
            xperms.push(Xperm {
                low: 0x0000,
                high: 0xFFFF,
                reset,
            });
        }
        Some(Token::HX(0)) => {
            tokens.next();
            xperms.push(Xperm {
                low: 0x0000,
                high: 0xFFFF,
                reset: true,
            });
        }
        Some(Token::HX(_)) => {
            let mut xperm = parse_xperm(tokens)?;
            xperm.reset = reset;
            xperms.push(xperm);
        }
        _ => throw!(),
    }
    Ok(xperms)
}

fn match_string<'a>(tokens: &mut Tokens<'a>, pattern: &str) -> ParseResult<'a, ()> {
    if let Some(Token::ID(s)) = tokens.next()
        && s == pattern
    {
        return Ok(());
    }
    Err(ParseError::General)
}

fn extract_token<'a>(s: &'a str, tokens: &mut Vec<Token<'a>>) {
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
        "*" => tokens.push(Token::ST),
        "-" => tokens.push(Token::HP),
        "" => {}
        _ => {
            if let Some(idx) = s.find('{') {
                let (a, b) = s.split_at(idx);
                extract_token(a, tokens);
                tokens.push(Token::LB);
                extract_token(&b[1..], tokens);
            } else if let Some(idx) = s.find('}') {
                let (a, b) = s.split_at(idx);
                extract_token(a, tokens);
                tokens.push(Token::RB);
                extract_token(&b[1..], tokens);
            } else if let Some(idx) = s.find(',') {
                let (a, b) = s.split_at(idx);
                extract_token(a, tokens);
                tokens.push(Token::CM);
                extract_token(&b[1..], tokens);
            } else if let Some(s) = s.strip_prefix('~') {
                tokens.push(Token::TL);
                extract_token(s, tokens);
            } else if let Some(idx) = s.find('-')
                && parse_xperm_hex(&s[..idx]).is_some()
            {
                let (a, b) = s.split_at(idx);
                extract_token(a, tokens);
                tokens.push(Token::HP);
                extract_token(&b[1..], tokens);
            } else if let Some(n) = parse_xperm_hex(s) {
                tokens.push(Token::HX(n));
            } else {
                tokens.push(Token::ID(s));
            }
        }
    }
}

fn tokenize_statement(statement: &str) -> Vec<Token<'_>> {
    let mut tokens = Vec::new();
    for s in statement.split_whitespace() {
        extract_token(s, &mut tokens);
    }
    tokens
}

// statement ::= AL sterm(s) sterm(t) sterm(c) sterm(p) { PolicyStatement::Allow };
// statement ::= DN sterm(s) sterm(t) sterm(c) sterm(p) { PolicyStatement::Deny };
// statement ::= AA sterm(s) sterm(t) sterm(c) sterm(p) { PolicyStatement::AuditAllow };
// statement ::= DA sterm(s) sterm(t) sterm(c) sterm(p) { PolicyStatement::DontAudit };
// statement ::= AX sterm(s) sterm(t) sterm(c) ID(i) xperms(p) { PolicyStatement::AllowXperm };
// statement ::= AY sterm(s) sterm(t) sterm(c) ID(i) xperms(p) { PolicyStatement::AuditAllowXperm };
// statement ::= DX sterm(s) sterm(t) sterm(c) ID(i) xperms(p) { PolicyStatement::DontAuditXperm };
// statement ::= PM sterm(t) { PolicyStatement::Permissive };
// statement ::= EF sterm(t) { PolicyStatement::Enforce };
// statement ::= TA term(t) term(a) { PolicyStatement::TypeAttribute };
// statement ::= TY ID(t) { PolicyStatement::Type };
// statement ::= TY ID(t) term(a) { PolicyStatement::Type };
// statement ::= AT ID(t) { PolicyStatement::Attribute };
// statement ::= TT ID(s) ID(t) ID(c) ID(d) { PolicyStatement::TypeTransition };
// statement ::= TT ID(s) ID(t) ID(c) ID(d) ID(o) { PolicyStatement::TypeTransition };
// statement ::= TC ID(s) ID(t) ID(c) ID(d) { PolicyStatement::TypeChange };
// statement ::= TM ID(s) ID(t) ID(c) ID(d) { PolicyStatement::TypeMember };
// statement ::= GF ID(s) ID(t) ID(c) { PolicyStatement::GenfsCon };
fn parse_statement_tokens<'a>(tokens: &mut Tokens<'a>) -> ParseResult<'a, PolicyStatement<'a>> {
    let action = match tokens.next() {
        Some(token) => token,
        _ => Err(ParseError::ShowHelp)?,
    };
    let check_additional_args = |tokens: &mut Tokens<'a>| {
        if tokens.peek().is_none() {
            Ok(())
        } else {
            Err(ParseError::General)
        }
    };
    match action {
        Token::AL | Token::DN | Token::AA | Token::DA => {
            let result = || -> ParseResult<PolicyStatement<'a>> {
                let src = parse_sterm(tokens)?;
                let target = parse_sterm(tokens)?;
                let class = parse_sterm(tokens)?;
                let perm = parse_sterm(tokens)?;
                check_additional_args(tokens)?;
                let stmt = match action {
                    Token::AL => PolicyStatement::Allow {
                        src,
                        target,
                        class,
                        perm,
                    },
                    Token::DN => PolicyStatement::Deny {
                        src,
                        target,
                        class,
                        perm,
                    },
                    Token::AA => PolicyStatement::AuditAllow {
                        src,
                        target,
                        class,
                        perm,
                    },
                    Token::DA => PolicyStatement::DontAudit {
                        src,
                        target,
                        class,
                        perm,
                    },
                    _ => unreachable!(),
                };
                Ok(stmt)
            }();
            result.map_err(|_| ParseError::AvtabAv(action))
        }
        Token::AX | Token::AY | Token::DX => {
            let result = || -> ParseResult<PolicyStatement<'a>> {
                let src = parse_sterm(tokens)?;
                let target = parse_sterm(tokens)?;
                let class = parse_sterm(tokens)?;
                match_string(tokens, "ioctl")?;
                let xperm = parse_xperms(tokens)?;
                check_additional_args(tokens)?;
                let stmt = match action {
                    Token::AX => PolicyStatement::AllowXperm {
                        src,
                        target,
                        class,
                        xperm,
                    },
                    Token::AY => PolicyStatement::AuditAllowXperm {
                        src,
                        target,
                        class,
                        xperm,
                    },
                    Token::DX => PolicyStatement::DontAuditXperm {
                        src,
                        target,
                        class,
                        xperm,
                    },
                    _ => unreachable!(),
                };
                Ok(stmt)
            }();
            result.map_err(|_| ParseError::AvtabXperms(action))
        }
        Token::PM | Token::EF => {
            let result = || -> ParseResult<PolicyStatement<'a>> {
                let type_ = parse_sterm(tokens)?;
                check_additional_args(tokens)?;
                let stmt = match action {
                    Token::PM => PolicyStatement::Permissive { type_ },
                    Token::EF => PolicyStatement::Enforce { type_ },
                    _ => unreachable!(),
                };
                Ok(stmt)
            }();
            result.map_err(|_| ParseError::TypeState(action))
        }
        Token::TA => {
            let result = || -> ParseResult<PolicyStatement<'a>> {
                let type_ = parse_term(tokens)?;
                let attr = parse_term(tokens)?;
                check_additional_args(tokens)?;
                Ok(PolicyStatement::TypeAttribute { type_, attr })
            }();
            result.map_err(|_| ParseError::TypeAttr)
        }
        Token::TY => {
            let result = || -> ParseResult<PolicyStatement<'a>> {
                let type_ = parse_id(tokens)?;
                let attr = if tokens.peek().is_none() {
                    vec![]
                } else {
                    parse_term(tokens)?
                };
                check_additional_args(tokens)?;
                Ok(PolicyStatement::Type { type_, attr })
            }();
            result.map_err(|_| ParseError::NewType)
        }
        Token::AT => {
            let result = || -> ParseResult<PolicyStatement<'a>> {
                let attr = parse_id(tokens)?;
                check_additional_args(tokens)?;
                Ok(PolicyStatement::Attribute { attr })
            }();
            result.map_err(|_| ParseError::NewAttr)
        }
        Token::TC | Token::TM => {
            let result = || -> ParseResult<PolicyStatement<'a>> {
                let src = parse_id(tokens)?;
                let target = parse_id(tokens)?;
                let class = parse_id(tokens)?;
                let default = parse_id(tokens)?;
                check_additional_args(tokens)?;
                let stmt = match action {
                    Token::TC => PolicyStatement::TypeChange {
                        src,
                        target,
                        class,
                        default,
                    },
                    Token::TM => PolicyStatement::TypeMember {
                        src,
                        target,
                        class,
                        default,
                    },
                    _ => unreachable!(),
                };
                Ok(stmt)
            }();
            result.map_err(|_| ParseError::AvtabType(action))
        }
        Token::TT => {
            let result = || -> ParseResult<PolicyStatement<'a>> {
                let src = parse_id(tokens)?;
                let target = parse_id(tokens)?;
                let class = parse_id(tokens)?;
                let default = parse_id(tokens)?;
                let object = if tokens.peek().is_none() {
                    ""
                } else {
                    parse_id(tokens)?
                };
                check_additional_args(tokens)?;
                Ok(PolicyStatement::TypeTransition {
                    src,
                    target,
                    class,
                    default,
                    object,
                })
            }();
            result.map_err(|_| ParseError::TypeTrans)
        }
        Token::GF => {
            let result = || -> ParseResult<PolicyStatement<'a>> {
                let fs_name = parse_id(tokens)?;
                let path = parse_id(tokens)?;
                let context = parse_id(tokens)?;
                check_additional_args(tokens)?;
                Ok(PolicyStatement::GenfsCon {
                    fs_name,
                    path,
                    context,
                })
            }();
            result.map_err(|_| ParseError::GenfsCon)
        }
        _ => Err(ParseError::UnknownAction(action)),
    }
}

impl SePolicy {
    pub fn load_rules(&mut self, rules: &str) {
        let mut cursor = Cursor::new(rules.as_bytes());
        self.load_rules_from_reader(&mut cursor);
    }

    pub fn load_rule_file(&mut self, filename: &Utf8CStr) {
        let result = || -> LoggedResult<()> {
            let file = filename.open(OFlag::O_RDONLY | OFlag::O_CLOEXEC)?;
            let mut reader = BufReader::new(file);
            self.load_rules_from_reader(&mut reader);
            Ok(())
        }();
        result.ok();
    }

    fn load_rules_from_reader<T: BufRead>(&mut self, reader: &mut T) {
        reader.for_each_line(|line| {
            self.parse_statement(line);
            true
        });
    }

    fn parse_statement(&mut self, statement: &str) {
        let statement = statement.trim();
        if statement.is_empty() || statement.starts_with('#') {
            return;
        }
        let mut tokens = tokenize_statement(statement).into_iter().peekable();
        let result = parse_statement_tokens(&mut tokens);
        match result {
            Ok(stmt) => self.exec_statement(stmt),
            Err(e) => {
                warn!("Syntax error in: \"{}\"", statement);
                error!("Hint: {}", e);
            }
        }
    }

    fn exec_statement(&mut self, statement: PolicyStatement<'_>) {
        match statement {
            PolicyStatement::Allow {
                src,
                target,
                class,
                perm,
            } => {
                self.allow(src, target, class, perm);
            }
            PolicyStatement::Deny {
                src,
                target,
                class,
                perm,
            } => {
                self.deny(src, target, class, perm);
            }
            PolicyStatement::AuditAllow {
                src,
                target,
                class,
                perm,
            } => {
                self.auditallow(src, target, class, perm);
            }
            PolicyStatement::DontAudit {
                src,
                target,
                class,
                perm,
            } => {
                self.dontaudit(src, target, class, perm);
            }
            PolicyStatement::AllowXperm {
                src,
                target,
                class,
                xperm,
            } => {
                self.allowxperm(src, target, class, xperm);
            }
            PolicyStatement::AuditAllowXperm {
                src,
                target,
                class,
                xperm,
            } => {
                self.auditallowxperm(src, target, class, xperm);
            }
            PolicyStatement::DontAuditXperm {
                src,
                target,
                class,
                xperm,
            } => {
                self.dontauditxperm(src, target, class, xperm);
            }
            PolicyStatement::Permissive { type_ } => {
                self.permissive(type_);
            }
            PolicyStatement::Enforce { type_ } => {
                self.enforce(type_);
            }
            PolicyStatement::TypeAttribute { type_, attr } => {
                self.typeattribute(type_, attr);
            }
            PolicyStatement::Type { type_, attr } => {
                self.type_(type_, attr);
            }
            PolicyStatement::Attribute { attr } => {
                self.attribute(attr);
            }
            PolicyStatement::TypeTransition {
                src,
                target,
                class,
                default,
                object,
            } => {
                self.type_transition(src, target, class, default, object);
            }
            PolicyStatement::TypeChange {
                src,
                target,
                class,
                default,
            } => {
                self.type_change(src, target, class, default);
            }
            PolicyStatement::TypeMember {
                src,
                target,
                class,
                default,
            } => {
                self.type_member(src, target, class, default);
            }
            PolicyStatement::GenfsCon {
                fs_name,
                path,
                context,
            } => {
                self.genfscon(fs_name, path, context);
            }
        }
    }
}

// Token to string
impl Display for Token<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            Token::AL => f.write_str("allow"),
            Token::DN => f.write_str("deny"),
            Token::AA => f.write_str("auditallow"),
            Token::DA => f.write_str("dontaudit"),
            Token::AX => f.write_str("allowxperm"),
            Token::AY => f.write_str("auditallowxperm"),
            Token::DX => f.write_str("dontauditxperm"),
            Token::PM => f.write_str("permissive"),
            Token::EF => f.write_str("enforce"),
            Token::TA => f.write_str("typeattribute"),
            Token::TY => f.write_str("type"),
            Token::AT => f.write_str("attribute"),
            Token::TT => f.write_str("type_transition"),
            Token::TC => f.write_str("type_change"),
            Token::TM => f.write_str("type_member"),
            Token::GF => f.write_str("genfscon"),
            Token::LB => f.write_char('{'),
            Token::RB => f.write_char('}'),
            Token::CM => f.write_char(','),
            Token::ST => f.write_char('*'),
            Token::TL => f.write_char('~'),
            Token::HP => f.write_char('-'),
            Token::HX(n) => f.write_fmt(format_args!("{n:06X}")),
            Token::ID(s) => f.write_str(s),
        }
    }
}

impl Display for ParseError<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            ParseError::General => Ok(()),
            ParseError::ShowHelp => format_statement_help(f),
            ParseError::AvtabAv(action) => {
                write!(f, "{action} *source_type *target_type *class *perm_set")
            }
            ParseError::AvtabXperms(action) => {
                write!(
                    f,
                    "{action} *source_type *target_type *class operation xperm_set"
                )
            }
            ParseError::AvtabType(action) => {
                write!(f, "{action} source_type target_type class default_type")
            }
            ParseError::TypeState(action) => {
                write!(f, "{action} *type")
            }
            ParseError::TypeAttr => f.write_str("typeattribute ^type ^attribute"),
            ParseError::TypeTrans => f.write_str(
                "type_transition source_type target_type class default_type (object_name)",
            ),
            ParseError::NewType => f.write_str("type type_name ^(attribute)"),
            ParseError::NewAttr => f.write_str("attribute attribute_name"),
            ParseError::GenfsCon => f.write_str("genfscon fs_name partial_path fs_context"),
            ParseError::UnknownAction(action) => write!(f, "Unknown action: \"{action}\""),
        }
    }
}

pub(crate) fn format_statement_help(f: &mut dyn Write) -> std::fmt::Result {
    write!(
        f,
        r#"** Policy statements:

One policy statement should be treated as a single parameter;
this means each policy statement should be enclosed in quotes.
Multiple policy statements can be provided in a single command.

Statements has a format of "<rule_name> [args...]".
Arguments labeled with (^) can accept one or more entries.
Multiple entries consist of a space separated list enclosed in braces ({{}}).
Arguments labeled with (*) are the same as (^), but additionally
support the match-all operator (*).

Example: "allow {{ s1 s2 }} {{ t1 t2 }} class *"
Will be expanded to:

allow s1 t1 class {{ all-permissions-of-class }}
allow s1 t2 class {{ all-permissions-of-class }}
allow s2 t1 class {{ all-permissions-of-class }}
allow s2 t2 class {{ all-permissions-of-class }}

** Extended permissions:

The only supported operation for extended permissions right now is 'ioctl'.
xperm_set is one or multiple hexadecimal numeric values ranging from 0x0000 to 0xFFFF.
Multiple values consist of a space separated list enclosed in braces ({{}}).
Use the complement operator (~) to specify all permissions except those explicitly listed.
Use the range operator (-) to specify all permissions within the low – high range.
Use the match all operator (*) to match all ioctl commands (0x0000-0xFFFF).
The special value 0 is used to clear all rules.

Some examples:
allowxperm source target class ioctl 0x8910
allowxperm source target class ioctl {{ 0x8910-0x8926 0x892A-0x8935 }}
allowxperm source target class ioctl ~{{ 0x8910 0x892A }}
allowxperm source target class ioctl *

** Supported policy statements:

{}
{}
{}
{}
{}
{}
{}
{}
{}
{}
{}
{}
{}
{}
{}
{}
"#,
        ParseError::AvtabAv(Token::AL),
        ParseError::AvtabAv(Token::DN),
        ParseError::AvtabAv(Token::AA),
        ParseError::AvtabAv(Token::DA),
        ParseError::AvtabXperms(Token::AX),
        ParseError::AvtabXperms(Token::AY),
        ParseError::AvtabXperms(Token::DX),
        ParseError::TypeState(Token::PM),
        ParseError::TypeState(Token::EF),
        ParseError::TypeAttr,
        ParseError::NewType,
        ParseError::NewAttr,
        ParseError::TypeTrans,
        ParseError::AvtabType(Token::TC),
        ParseError::AvtabType(Token::TM),
        ParseError::GenfsCon
    )
}

#[cfg(test)]
mod tests {
    use super::*;

    fn parse(statement: &str) -> ParseResult<PolicyStatement> {
        let mut tokens = tokenize_statement(statement).into_iter().peekable();
        parse_statement_tokens(&mut tokens)
    }

    #[test]
    fn test_parse_allow() {
        assert_eq!(
            parse("allow s t c p"),
            Ok(PolicyStatement::Allow {
                src: vec!["s"],
                target: vec!["t"],
                class: vec!["c"],
                perm: vec!["p"],
            })
        );

        assert_eq!(
            parse("allow { s1 s2 } { t1 t2 } { c1 c2 } { p1 p2 }"),
            Ok(PolicyStatement::Allow {
                src: vec!["s1", "s2"],
                target: vec!["t1", "t2"],
                class: vec!["c1", "c2"],
                perm: vec!["p1", "p2"],
            })
        );

        assert_eq!(
            parse("allow * * * *"),
            Ok(PolicyStatement::Allow {
                src: vec![],
                target: vec![],
                class: vec![],
                perm: vec![],
            })
        );
    }

    #[test]
    fn test_parse_deny_auditallow_dontaudit() {
        assert_eq!(
            parse("deny s t c p"),
            Ok(PolicyStatement::Deny {
                src: vec!["s"],
                target: vec!["t"],
                class: vec!["c"],
                perm: vec!["p"],
            })
        );

        assert_eq!(
            parse("auditallow s t c p"),
            Ok(PolicyStatement::AuditAllow {
                src: vec!["s"],
                target: vec!["t"],
                class: vec!["c"],
                perm: vec!["p"],
            })
        );

        assert_eq!(
            parse("dontaudit s t c p"),
            Ok(PolicyStatement::DontAudit {
                src: vec!["s"],
                target: vec!["t"],
                class: vec!["c"],
                perm: vec!["p"],
            })
        );
    }

    #[test]
    fn test_parse_xperms() {
        assert_eq!(
            parse("allowxperm s t c ioctl 0x8910"),
            Ok(PolicyStatement::AllowXperm {
                src: vec!["s"],
                target: vec!["t"],
                class: vec!["c"],
                xperm: vec![Xperm {
                    low: 0x8910,
                    high: 0x8910,
                    reset: false,
                }],
            })
        );

        assert_eq!(
            parse("auditallowxperm s t c ioctl 0x8910-0x8926"),
            Ok(PolicyStatement::AuditAllowXperm {
                src: vec!["s"],
                target: vec!["t"],
                class: vec!["c"],
                xperm: vec![Xperm {
                    low: 0x8910,
                    high: 0x8926,
                    reset: false,
                }],
            })
        );

        assert_eq!(
            parse("auditallowxperm s t c ioctl 0x8910 - 0x8926"),
            Ok(PolicyStatement::AuditAllowXperm {
                src: vec!["s"],
                target: vec!["t"],
                class: vec!["c"],
                xperm: vec![Xperm {
                    low: 0x8910,
                    high: 0x8926,
                    reset: false,
                }],
            })
        );

        assert_eq!(
            parse("allowxperm s t c ioctl { 0x8910-0x8926 0x892A-0x8935 }"),
            Ok(PolicyStatement::AllowXperm {
                src: vec!["s"],
                target: vec!["t"],
                class: vec!["c"],
                xperm: vec![
                    Xperm {
                        low: 0x8910,
                        high: 0x8926,
                        reset: false,
                    },
                    Xperm {
                        low: 0x892A,
                        high: 0x8935,
                        reset: false,
                    },
                ],
            })
        );

        assert_eq!(
            parse("dontauditxperm s t c ioctl ~{ 0x8910 0x892A }"),
            Ok(PolicyStatement::DontAuditXperm {
                src: vec!["s"],
                target: vec!["t"],
                class: vec!["c"],
                xperm: vec![
                    Xperm {
                        low: 0x8910,
                        high: 0x8910,
                        reset: true,
                    },
                    Xperm {
                        low: 0x892A,
                        high: 0x892A,
                        reset: true,
                    },
                ],
            })
        );

        assert_eq!(
            parse("allowxperm s t c ioctl *"),
            Ok(PolicyStatement::AllowXperm {
                src: vec!["s"],
                target: vec!["t"],
                class: vec!["c"],
                xperm: vec![Xperm {
                    low: 0x0000,
                    high: 0xFFFF,
                    reset: false,
                }],
            })
        );
    }

    #[test]
    fn test_parse_permissive_enforce() {
        assert_eq!(
            parse("permissive t"),
            Ok(PolicyStatement::Permissive { type_: vec!["t"] })
        );

        assert_eq!(
            parse("enforce { t1 t2 }"),
            Ok(PolicyStatement::Enforce {
                type_: vec!["t1", "t2"]
            })
        );
    }

    #[test]
    fn test_parse_type_and_attribute() {
        assert_eq!(
            parse("typeattribute t { a1 a2 }"),
            Ok(PolicyStatement::TypeAttribute {
                type_: vec!["t"],
                attr: vec!["a1", "a2"],
            })
        );

        assert_eq!(
            parse("type my_type"),
            Ok(PolicyStatement::Type {
                type_: "my_type",
                attr: vec![],
            })
        );

        assert_eq!(
            parse("type my_type { a1 a2 }"),
            Ok(PolicyStatement::Type {
                type_: "my_type",
                attr: vec!["a1", "a2"],
            })
        );

        assert_eq!(
            parse("attribute my_attr"),
            Ok(PolicyStatement::Attribute { attr: "my_attr" })
        );
    }

    #[test]
    fn test_parse_transitions_and_members() {
        assert_eq!(
            parse("type_transition s t c d"),
            Ok(PolicyStatement::TypeTransition {
                src: "s",
                target: "t",
                class: "c",
                default: "d",
                object: "",
            })
        );

        assert_eq!(
            parse("type_transition s t c d o"),
            Ok(PolicyStatement::TypeTransition {
                src: "s",
                target: "t",
                class: "c",
                default: "d",
                object: "o",
            })
        );

        assert_eq!(
            parse("type_change s t c d"),
            Ok(PolicyStatement::TypeChange {
                src: "s",
                target: "t",
                class: "c",
                default: "d",
            })
        );

        assert_eq!(
            parse("type_member s t c d"),
            Ok(PolicyStatement::TypeMember {
                src: "s",
                target: "t",
                class: "c",
                default: "d",
            })
        );
    }

    #[test]
    fn test_parse_genfscon() {
        assert_eq!(
            parse("genfscon sysfs /fs_context context_t"),
            Ok(PolicyStatement::GenfsCon {
                fs_name: "sysfs",
                path: "/fs_context",
                context: "context_t",
            })
        );
    }

    #[test]
    fn test_parse_syntax_errors_and_corner_cases() {
        // Unknown action
        assert!(matches!(
            parse("unknown_action s t c p"),
            Err(ParseError::UnknownAction(_))
        ));

        // Missing arguments
        assert!(matches!(parse("allow s t c"), Err(ParseError::AvtabAv(_))));

        // Extra trailing arguments
        assert!(matches!(
            parse("allow s t c p extra"),
            Err(ParseError::AvtabAv(_))
        ));

        // Invalid xperm missing "ioctl"
        assert!(matches!(
            parse("allowxperm s t c 0x8910"),
            Err(ParseError::AvtabXperms(_))
        ));

        // Extra arguments on attribute
        assert!(matches!(
            parse("attribute a1 extra"),
            Err(ParseError::NewAttr)
        ));

        // Empty token iterator
        assert!(matches!(parse(""), Err(ParseError::ShowHelp)));

        // No spaces around braces and commas
        assert_eq!(
            parse("allow{s1 s2}{t1 t2}c p"),
            Ok(PolicyStatement::Allow {
                src: vec!["s1", "s2"],
                target: vec!["t1", "t2"],
                class: vec!["c"],
                perm: vec!["p"],
            })
        );

        assert_eq!(
            parse("allow {s1,s2} {t1,t2} c p"),
            Ok(PolicyStatement::Allow {
                src: vec!["s1", "s2"],
                target: vec!["t1", "t2"],
                class: vec!["c"],
                perm: vec!["p"],
            })
        );

        // Multiple spaces between tokens
        assert_eq!(
            parse("allow    s    t    c    p"),
            Ok(PolicyStatement::Allow {
                src: vec!["s"],
                target: vec!["t"],
                class: vec!["c"],
                perm: vec!["p"],
            })
        );

        assert_eq!(
            parse("allow   {   s1   s2   }   {   t1   t2   }   c   p"),
            Ok(PolicyStatement::Allow {
                src: vec!["s1", "s2"],
                target: vec!["t1", "t2"],
                class: vec!["c"],
                perm: vec!["p"],
            })
        );

        // Type names containing hyphens (issue #9819 regression)
        assert_eq!(
            parse("allow mslgd vendor_port-bridge dir { search }"),
            Ok(PolicyStatement::Allow {
                src: vec!["mslgd"],
                target: vec!["vendor_port-bridge"],
                class: vec!["dir"],
                perm: vec!["search"],
            })
        );
    }
}
