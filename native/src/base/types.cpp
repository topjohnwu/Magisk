module;
#include <rust/cxx.h>

export module base:types;
import std;
import :utils;

export template<__SIZE_TYPE__ N>
struct StringLiteral {
    char value[N]{};
    consteval StringLiteral() = default;
    consteval StringLiteral(const char (&text)[N]) {
        for (__SIZE_TYPE__ i = 0; i < N; ++i) value[i] = text[i];
    }
    constexpr const char *c_str() const [[clang::lifetimebound]] { return value; }
    constexpr operator const char *() const [[clang::lifetimebound]] { return value; }
    template<__SIZE_TYPE__ M>
    consteval auto operator+(const StringLiteral<M> &rhs) const {
        StringLiteral<N + M - 1> result;
        for (__SIZE_TYPE__ i = 0; i < N - 1; ++i) result.value[i] = value[i];
        for (__SIZE_TYPE__ i = 0; i < M; ++i) result.value[N - 1 + i] = rhs.value[i];
        return result;
    }
    template<__SIZE_TYPE__ M>
    consteval auto operator+(const char (&rhs)[M]) const {
        return *this + StringLiteral<M>(rhs);
    }
};

export template<__SIZE_TYPE__ N, __SIZE_TYPE__ M>
consteval auto operator+(const char (&lhs)[N], const StringLiteral<M> &rhs) {
    return StringLiteral<N>(lhs) + rhs;
}

export template<StringLiteral Text>
consteval const auto &operator""_cs() { return Text; }

export {
struct Utf8CStr;

// Bindings to &Utf8CStr in Rust
extern "C" void cxx$utf8str$new(Utf8CStr *self, const void *s, size_t len);
extern "C" const char *cxx$utf8str$ptr(const Utf8CStr *self);
extern "C" size_t cxx$utf8str$len(const Utf8CStr *self);

struct Utf8CStr {
    const char *data() const {
        return cxx$utf8str$ptr(this);
    }
    size_t length() const {
        return cxx$utf8str$len(this);
    }
    Utf8CStr(const char *s, size_t len) : repr{} {
        cxx$utf8str$new(this, s, len);
    }
    template<__SIZE_TYPE__ N>
    Utf8CStr(const StringLiteral<N> &s [[clang::lifetimebound]]) : Utf8CStr(s.c_str()) {}

    Utf8CStr() : Utf8CStr("", 1) {};
    Utf8CStr(const Utf8CStr &o) = default;
    Utf8CStr(const char *s) : Utf8CStr(s, strlen(s) + 1) {};
    Utf8CStr(std::string s) : Utf8CStr(s.data(), s.length() + 1) {};
    const char *c_str() const { return this->data(); }
    size_t size() const { return this->length(); }
    bool empty() const { return this->length() == 0 ; }
    std::string_view sv() const { return {data(), length()}; }
    operator std::string_view() const { return sv(); }
    bool operator==(std::string_view rhs) const { return sv() == rhs; }

private:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
    std::array<std::uintptr_t, 2> repr;
#pragma clang diagnostic pop
};

// Bindings for std::function to be callable from Rust
using CxxFnBoolStrStr = std::function<bool(rust::Str, rust::Str)>;
struct FnBoolStrStr : public CxxFnBoolStrStr {
    using CxxFnBoolStrStr::function;
    bool call(rust::Str a, rust::Str b) const {
        return operator()(a, b);
    }
};
using CxxFnBoolStr = std::function<bool(Utf8CStr)>;
struct FnBoolStr : public CxxFnBoolStr {
    using CxxFnBoolStr::function;
    bool call(Utf8CStr s) const {
        return operator()(s);
    }
};

}
