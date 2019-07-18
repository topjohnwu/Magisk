#pragma once

#define UID_SHELL  (get_shell_uid())
#define UID_ROOT   0

#ifdef __cplusplus
extern "C" {
#endif

unsigned get_shell_uid();
int fork_dont_care();
int fork_no_zombie();
int strend(const char *s1, const char *s2);
char *rtrim(char *str);
void init_argv0(int argc, char **argv);
void set_nice_name(const char *name);
int parse_int(const char *s);

#ifdef __cplusplus
}

#include <string>
#include <functional>
#include <string_view>

void gen_rand_str(char *buf, int len, bool varlen = true);

#define str_contains(s, ss) ((ss) != nullptr && (s).find(ss) != std::string::npos)
#define str_starts(s, ss) ((ss) != nullptr && (s).compare(0, strlen(ss), ss) == 0)

class MutexGuard {
public:
	explicit MutexGuard(pthread_mutex_t &m): mutex(&m) {
		pthread_mutex_lock(mutex);
	}

	explicit MutexGuard(pthread_mutex_t *m): mutex(m) {
		pthread_mutex_lock(mutex);
	}

	~MutexGuard() {
		pthread_mutex_unlock(mutex);
	}

private:
	pthread_mutex_t *mutex;
};

class RunFinally {
public:
	explicit RunFinally(std::function<void()> &&fn): fn(std::move(fn)) {}

	void disable() { fn = nullptr; }

	~RunFinally() { if (fn) fn(); }

private:
	std::function<void ()> fn;
};

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

#endif