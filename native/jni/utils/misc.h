#pragma once

#include <pthread.h>
#include <string>
#include <functional>
#include <string_view>

#define UID_ROOT   0
#define UID_SHELL  2000

#define str_contains(s, ss) ((ss) != nullptr && (s).find(ss) != std::string::npos)
#define str_starts(s, ss) ((ss) != nullptr && (s).compare(0, strlen(ss), ss) == 0)

class mutex_guard {
public:
	explicit mutex_guard(pthread_mutex_t &m): mutex(&m) {
		pthread_mutex_lock(mutex);
	}

	explicit mutex_guard(pthread_mutex_t *m): mutex(m) {
		pthread_mutex_lock(mutex);
	}

	~mutex_guard() {
		pthread_mutex_unlock(mutex);
	}

private:
	pthread_mutex_t *mutex;
};

class run_finally {
public:
	explicit run_finally(std::function<void()> &&fn): fn(std::move(fn)) {}

	void disable() { fn = nullptr; }

	~run_finally() { if (fn) fn(); }

private:
	std::function<void()> fn;
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

int parse_int(const char *s);
static inline int parse_int(std::string s) { return parse_int(s.data()); }
static inline int parse_int(std::string_view s) { return parse_int(s.data()); }

int new_daemon_thread(void *(*start_routine) (void *), void *arg = nullptr,
					  const pthread_attr_t *attr = nullptr);
int new_daemon_thread(std::function<void()> &&fn);

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
	exec_t exec{};
	return exec_command_sync(exec, args...);
}

bool ends_with(const std::string_view &s1, const std::string_view &s2);
int fork_dont_care();
int fork_no_zombie();
int strend(const char *s1, const char *s2);
char *rtrim(char *str);
void init_argv0(int argc, char **argv);
void set_nice_name(const char *name);
uint32_t binary_gcd(uint32_t u, uint32_t v);
int switch_mnt_ns(int pid);
void gen_rand_str(char *buf, int len, bool varlen = true);
