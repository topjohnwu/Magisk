#pragma once

#include <pthread.h>
#include <string>
#include <functional>
#include <string_view>
#include <bitset>
#include <compare>
#include "xwrap.hpp"

#define UID_ROOT   0
#define UID_SHELL  2000

#define DISALLOW_COPY_AND_MOVE(clazz) \
clazz(const clazz &) = delete; \
clazz(clazz &&) = delete;

#define to_app_id(uid)  (uid % 100000)
#define to_user_id(uid) (uid / 100000)

class mutex_guard {
    DISALLOW_COPY_AND_MOVE(mutex_guard)
public:
    explicit mutex_guard(pthread_mutex_t &m): mutex(&m) {
        pthread_mutex_lock(mutex);
    }
    void unlock() {
        pthread_mutex_unlock(mutex);
        mutex = nullptr;
    }
    ~mutex_guard() {
        if (mutex) pthread_mutex_unlock(mutex);
    }
private:
    pthread_mutex_t *mutex;
};

template <class Func>
class run_finally {
    DISALLOW_COPY_AND_MOVE(run_finally)
public:
    explicit run_finally(Func &&fn) : fn(std::move(fn)) {}
    ~run_finally() { fn(); }
private:
    Func fn;
};

template <typename T>
class reversed_container {
public:
    reversed_container(T &base) : base(base) {}
    decltype(std::declval<T>().rbegin()) begin() { return base.rbegin(); }
    decltype(std::declval<T>().crbegin()) begin() const { return base.crbegin(); }
    decltype(std::declval<T>().crbegin()) cbegin() const { return base.crbegin(); }
    decltype(std::declval<T>().rend()) end() { return base.rend(); }
    decltype(std::declval<T>().crend()) end() const { return base.crend(); }
    decltype(std::declval<T>().crend()) cend() const { return base.crend(); }
private:
    T &base;
};

template <typename T>
reversed_container<T> reversed(T &base) {
    return reversed_container<T>(base);
}

template<class T>
static inline void default_new(T *&p) { p = new T(); }

template<class T>
static inline void default_new(std::unique_ptr<T> &p) { p.reset(new T()); }

template<typename T, typename Impl>
class stateless_allocator {
public:
    using value_type = T;
    T *allocate(size_t num) { return static_cast<T*>(Impl::allocate(sizeof(T) * num)); }
    void deallocate(T *ptr, size_t num) { Impl::deallocate(ptr, sizeof(T) * num); }
    stateless_allocator()                           = default;
    stateless_allocator(const stateless_allocator&) = default;
    stateless_allocator(stateless_allocator&&)      = default;
    template <typename U>
    stateless_allocator(const stateless_allocator<U, Impl>&) {}
    bool operator==(const stateless_allocator&) { return true; }
    bool operator!=(const stateless_allocator&) { return false; }
};

class dynamic_bitset_impl {
public:
    using slot_type = unsigned long;
    constexpr static int slot_size = sizeof(slot_type) * 8;
    using slot_bits = std::bitset<slot_size>;

    size_t slots() const { return slot_list.size(); }
    slot_type get_slot(size_t slot) const {
        return slot_list.size() > slot ? slot_list[slot].to_ulong() : 0ul;
    }
    void emplace_back(slot_type l) {
        slot_list.emplace_back(l);
    }
protected:
    slot_bits::reference get(size_t pos) {
        size_t slot = pos / slot_size;
        size_t index = pos % slot_size;
        if (slot_list.size() <= slot) {
            slot_list.resize(slot + 1);
        }
        return slot_list[slot][index];
    }
    bool get(size_t pos) const {
        size_t slot = pos / slot_size;
        size_t index = pos % slot_size;
        return slot_list.size() > slot && slot_list[slot][index];
    }
private:
    std::vector<slot_bits> slot_list;
};

struct dynamic_bitset : public dynamic_bitset_impl {
    slot_bits::reference operator[] (size_t pos) { return get(pos); }
    bool operator[] (size_t pos) const { return get(pos); }
};

struct StringCmp {
    using is_transparent = void;
    bool operator()(std::string_view a, std::string_view b) const { return a < b; }
};

int parse_int(std::string_view s);

using thread_entry = void *(*)(void *);
int new_daemon_thread(thread_entry entry, void *arg = nullptr);

static inline bool str_contains(std::string_view s, std::string_view ss) {
    return s.find(ss) != std::string::npos;
}
static inline bool str_starts(std::string_view s, std::string_view ss) {
    return s.size() >= ss.size() && s.compare(0, ss.size(), ss) == 0;
}
static inline bool str_ends(std::string_view s, std::string_view ss) {
    return s.size() >= ss.size() && s.compare(s.size() - ss.size(), std::string::npos, ss) == 0;
}
static inline std::string ltrim(std::string &&s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    return std::move(s);
}
static inline std::string rtrim(std::string &&s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch) && ch != '\0';
    }).base(), s.end());
    return std::move(s);
}

int fork_dont_care();
int fork_no_orphan();
void init_argv0(int argc, char **argv);
void set_nice_name(const char *name);
uint32_t binary_gcd(uint32_t u, uint32_t v);
int switch_mnt_ns(int pid);
int gen_rand_str(char *buf, int len, bool varlen = true);
std::string &replace_all(std::string &str, std::string_view from, std::string_view to);
std::vector<std::string> split(const std::string& s, const std::string& delimiters);

struct exec_t {
    bool err = false;
    int fd = -2;
    void (*pre_exec)() = nullptr;
    int (*fork)() = xfork;
    const char **argv = nullptr;
};

int exec_command(exec_t &exec);
template <class ...Args>
int exec_command(exec_t &exec, Args &&...args) {
    const char *argv[] = {args..., nullptr};
    exec.argv = argv;
    return exec_command(exec);
}
int exec_command_sync(exec_t &exec);
template <class ...Args>
int exec_command_sync(exec_t &exec, Args &&...args) {
    const char *argv[] = {args..., nullptr};
    exec.argv = argv;
    return exec_command_sync(exec);
}
template <class ...Args>
int exec_command_sync(Args &&...args) {
    exec_t exec;
    return exec_command_sync(exec, args...);
}
template <class ...Args>
void exec_command_async(Args &&...args) {
    const char *argv[] = {args..., nullptr};
    exec_t exec {
        .fork = fork_dont_care,
        .argv = argv,
    };
    exec_command(exec);
}

template <class _Tp, bool = std::is_enum_v<_Tp> >
struct __is_scoped_enum_helper : std::false_type {};

template <class _Tp>
struct __is_scoped_enum_helper<_Tp, true>
: public std::bool_constant<!std::is_convertible_v<_Tp, std::underlying_type_t<_Tp> > > {};

template <class _Tp>
struct is_scoped_enum
        : public __is_scoped_enum_helper<_Tp> {};

template <class _Tp>
inline constexpr bool is_scoped_enum_v = is_scoped_enum<_Tp>::value;

template<typename Enum> requires( is_scoped_enum_v<Enum> )
constexpr inline auto operator <=> (std::underlying_type_t<Enum> a, Enum b) {
    return a <=> static_cast<std::underlying_type_t<Enum>>(b);
}

template<typename Enum> requires( is_scoped_enum_v<Enum> )
constexpr inline auto operator != (std::underlying_type_t<Enum> a, Enum b) {
    return a != static_cast<std::underlying_type_t<Enum>>(b);
}

template<typename Enum> requires( is_scoped_enum_v<Enum> )
constexpr inline auto operator == (std::underlying_type_t<Enum> a, Enum b) {
    return a == static_cast<std::underlying_type_t<Enum>>(b);
}
