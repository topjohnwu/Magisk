#include <sys/types.h>
#include <sys/wait.h>

#include <base.hpp>
#include <selinux.hpp>

#include "su.hpp"

extern int SDK_INT;

using namespace std;

#define CALL_PROVIDER \
exe, "/system/bin", "com.android.commands.content.Content", \
"call", "--uri", target, "--user", user, "--method", action

#define START_ACTIVITY \
exe, "/system/bin", "com.android.commands.am.Am", \
"start", "-p", target, "--user", user, "-a", "android.intent.action.VIEW", \
"-f", "0x58800020", "--es", "action", action

// 0x58800020 = FLAG_ACTIVITY_NEW_TASK|FLAG_ACTIVITY_MULTIPLE_TASK|
//              FLAG_ACTIVITY_NO_HISTORY|FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS|
//              FLAG_INCLUDE_STOPPED_PACKAGES

#define get_cmd(to) \
((to).command.empty() ? \
((to).shell.empty() ? DEFAULT_SHELL : (to).shell.data()) : \
(to).command.data())

class Extra {
    const char *key;
    enum {
        INT,
        BOOL,
        STRING
    } type;
    union {
        int int_val;
        bool bool_val;
        const char *str_val;
    };
    string str;
public:
    Extra(const char *k, int v): key(k), type(INT), int_val(v) {}
    Extra(const char *k, bool v): key(k), type(BOOL), bool_val(v) {}
    Extra(const char *k, const char *v): key(k), type(STRING), str_val(v) {}

    void add_intent(vector<const char *> &vec) {
        char buf[32];
        const char *val;
        switch (type) {
        case INT:
            vec.push_back("--ei");
            ssprintf(buf, sizeof(buf), "%d", int_val);
            str = buf;
            val = str.data();
            break;
        case BOOL:
            vec.push_back("--ez");
            val = bool_val ? "true" : "false";
            break;
        case STRING:
            vec.push_back("--es");
            val = str_val;
            break;
        }
        vec.push_back(key);
        vec.push_back(val);
    }

    void add_bind(vector<const char *> &vec) {
        char buf[32];
        str = key;
        switch (type) {
        case INT:
            str += ":i:";
            ssprintf(buf, sizeof(buf), "%d", int_val);
            str += buf;
            break;
        case BOOL:
            str += ":b:";
            str += bool_val ? "true" : "false";
            break;
        case STRING:
            str += ":s:";
            if (SDK_INT >= 30) {
                string tmp = str_val;
                replace_all(tmp, "\\", "\\\\");
                replace_all(tmp, ":", "\\:");
                str += tmp;
            } else {
                str += str_val;
            }
            break;
        }
        vec.push_back("--extra");
        vec.push_back(str.data());
    }
};

static bool check_no_error(int fd) {
    char buf[1024];
    auto out = xopen_file(fd, "r");
    while (fgets(buf, sizeof(buf), out.get())) {
        if (strncmp(buf, "Error", 5) == 0)
            return false;
    }
    return true;
}

static void exec_cmd(const char *action, vector<Extra> &data,
                     const shared_ptr<su_info> &info, bool provider = true) {
    char exe[128];
    char target[128];
    char user[4];
    ssprintf(user, sizeof(user), "%d", to_user_id(info->eval_uid));

    if (zygisk_enabled) {
#if defined(__LP64__)
        ssprintf(exe, sizeof(exe), "/proc/self/fd/%d", app_process_64);
#else
        ssprintf(exe, sizeof(exe), "/proc/self/fd/%d", app_process_32);
#endif
    } else {
        strscpy(exe, "/system/bin/app_process", sizeof(exe));
    }

    // First try content provider call method
    if (provider) {
        ssprintf(target, sizeof(target), "content://%s.provider", info->mgr_pkg.data());
        vector<const char *> args{ CALL_PROVIDER };
        for (auto &e : data) {
            e.add_bind(args);
        }
        args.push_back(nullptr);
        exec_t exec {
            .err = true,
            .fd = -1,
            .pre_exec = [] { setenv("CLASSPATH", "/system/framework/content.jar", 1); },
            .argv = args.data()
        };
        exec_command_sync(exec);
        if (check_no_error(exec.fd))
            return;
    }

    vector<const char *> args{ START_ACTIVITY };
    for (auto &e : data) {
        e.add_intent(args);
    }
    args.push_back(nullptr);
    exec_t exec {
        .err = true,
        .fd = -1,
        .pre_exec = [] { setenv("CLASSPATH", "/system/framework/am.jar", 1); },
        .argv = args.data()
    };

    // Then try start activity without package name
    strscpy(target, info->mgr_pkg.data(), sizeof(target));
    exec_command_sync(exec);
    if (check_no_error(exec.fd))
        return;

    // Finally, fallback to start activity with component name
    args[4] = "-n";
    ssprintf(target, sizeof(target), "%s/com.topjohnwu.magisk.ui.surequest.SuRequestActivity", info->mgr_pkg.data());
    exec.fd = -2;
    exec.fork = fork_dont_care;
    exec_command(exec);
}

void app_log(const su_context &ctx) {
    if (fork_dont_care() == 0) {
        vector<Extra> extras;
        extras.reserve(6);
        extras.emplace_back("from.uid", ctx.info->uid);
        extras.emplace_back("to.uid", ctx.req.uid);
        extras.emplace_back("pid", ctx.pid);
        extras.emplace_back("policy", ctx.info->access.policy);
        extras.emplace_back("command", get_cmd(ctx.req));
        extras.emplace_back("notify", (bool) ctx.info->access.notify);

        exec_cmd("log", extras, ctx.info);
        exit(0);
    }
}

void app_notify(const su_context &ctx) {
    if (fork_dont_care() == 0) {
        vector<Extra> extras;
        extras.reserve(3);
        extras.emplace_back("from.uid", ctx.info->uid);
        extras.emplace_back("pid", ctx.pid);
        extras.emplace_back("policy", ctx.info->access.policy);

        exec_cmd("notify", extras, ctx.info);
        exit(0);
    }
}

int app_request(const su_context &ctx) {
    // Create FIFO
    char fifo[64];
    strcpy(fifo, "/dev/socket/");
    gen_rand_str(fifo + 12, 32, true);
    mkfifo(fifo, 0600);
    chown(fifo, ctx.info->mgr_uid, ctx.info->mgr_uid);
    setfilecon(fifo, "u:object_r:" SEPOL_FILE_TYPE ":s0");

    // Send request
    vector<Extra> extras;
    extras.reserve(3);
    extras.emplace_back("fifo", fifo);
    extras.emplace_back("uid", ctx.info->eval_uid);
    extras.emplace_back("pid", ctx.pid);
    exec_cmd("request", extras, ctx.info, false);

    // Wait for data input for at most 70 seconds
    // Open with O_RDWR to prevent FIFO open block
    int fd = xopen(fifo, O_RDWR | O_CLOEXEC);
    struct pollfd pfd = {
        .fd = fd,
        .events = POLLIN
    };
    if (xpoll(&pfd, 1, 70 * 1000) <= 0) {
        close(fd);
        fd = -1;
    }

    unlink(fifo);
    return fd;
}
