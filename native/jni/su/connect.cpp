#include <sys/types.h>
#include <sys/wait.h>

#include <utils.hpp>
#include <selinux.hpp>

#include "su.hpp"

using namespace std;

enum {
    NAMED_ACTIVITY,
    PKG_ACTIVITY,
    CONTENT_PROVIDER
};

#define CALL_PROVIDER \
"/system/bin/app_process", "/system/bin", "com.android.commands.content.Content", \
"call", "--uri", target, "--user", user, "--method", action

#define START_ACTIVITY \
"/system/bin/app_process", "/system/bin", "com.android.commands.am.Am", \
"start", "-p", target, "--user", user, "-a", "android.intent.action.VIEW", \
"-f", "0x18000020", "--es", "action", action

// 0x18000020 = FLAG_ACTIVITY_NEW_TASK|FLAG_ACTIVITY_MULTIPLE_TASK|FLAG_INCLUDE_STOPPED_PACKAGES

#define get_user(info) \
(info->cfg[SU_MULTIUSER_MODE] == MULTIUSER_MODE_USER \
? info->uid / 100000 : 0)

#define get_cmd(to) \
(to.command.empty() ? (to.shell.empty() ? DEFAULT_SHELL : to.shell.data()) : to.command.data())

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
    char buf[32];
public:
    Extra(const char *k, int v): key(k), type(INT), int_val(v) {}
    Extra(const char *k, bool v): key(k), type(BOOL), bool_val(v) {}
    Extra(const char *k, const char *v): key(k), type(STRING), str_val(v) {}

    void add_intent(vector<const char *> &vec) {
        const char *val;
        switch (type) {
            case INT:
                vec.push_back("--ei");
                sprintf(buf, "%d", int_val);
                val = buf;
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
        switch (type) {
            case INT:
                sprintf(buf, "%s:i:%d", key, int_val);
                break;
            case BOOL:
                sprintf(buf, "%s:b:%s", key, bool_val ? "true" : "false");
                break;
            case STRING:
                sprintf(buf, "%s:s:%s", key, str_val);
                break;
        }
        vec.push_back("--extra");
        vec.push_back(buf);
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
                     const shared_ptr<su_info> &info, int mode = CONTENT_PROVIDER) {
    char target[128];
    char user[4];
    sprintf(user, "%d", get_user(info));

    // First try content provider call method
    if (mode >= CONTENT_PROVIDER) {
        sprintf(target, "content://%s.provider", info->str[SU_MANAGER].data());
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

    if (mode >= PKG_ACTIVITY) {
        // Then try start activity without component name
        strcpy(target, info->str[SU_MANAGER].data());
        exec_command_sync(exec);
        if (check_no_error(exec.fd))
            return;
    }

    // Finally, fallback to start activity with component name
    args[4] = "-n";
    sprintf(target, "%s/.ui.surequest.SuRequestActivity", info->str[SU_MANAGER].data());
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
        extras.reserve(2);
        extras.emplace_back("from.uid", ctx.info->uid);
        extras.emplace_back("policy", ctx.info->access.policy);

        exec_cmd("notify", extras, ctx.info);
        exit(0);
    }
}

int app_request(const shared_ptr<su_info> &info) {
    // Create FIFO
    char fifo[64];
    strcpy(fifo, "/dev/socket/");
    gen_rand_str(fifo + 12, 32, true);
    mkfifo(fifo, 0600);
    chown(fifo, info->mgr_st.st_uid, info->mgr_st.st_gid);
    setfilecon(fifo, "u:object_r:" SEPOL_FILE_TYPE ":s0");

    // Send request
    vector<Extra> extras;
    extras.reserve(2);
    extras.emplace_back("fifo", fifo);
    extras.emplace_back("uid", info->uid);
    exec_cmd("request", extras, info, PKG_ACTIVITY);

    // Wait for data input for at most 70 seconds
    int fd = xopen(fifo, O_RDONLY | O_CLOEXEC);
    struct pollfd pfd = {
        .fd = fd,
        .events = POLL_IN
    };
    if (xpoll(&pfd, 1, 70 * 1000) <= 0) {
        close(fd);
        fd = -1;
    }

    unlink(fifo);
    return fd;
}
