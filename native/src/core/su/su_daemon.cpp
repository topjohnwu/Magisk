#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/select.h>
#include <sys/time.h>

#include <consts.hpp>
#include <base.hpp>
#include <selinux.hpp>

#include "su.hpp"
#include "pts.hpp"

using namespace std;

static pthread_mutex_t cache_lock = PTHREAD_MUTEX_INITIALIZER;
static shared_ptr<su_info> cached;

su_info::su_info(int uid) :
uid(uid), eval_uid(-1), access(DEFAULT_SU_ACCESS), mgr_uid(-1),
timestamp(0), _lock(PTHREAD_MUTEX_INITIALIZER) {}

su_info::~su_info() {
    pthread_mutex_destroy(&_lock);
}

mutex_guard su_info::lock() {
    return mutex_guard(_lock);
}

bool su_info::is_fresh() {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long current = ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
    return current - timestamp < 3000;  /* 3 seconds */
}

void su_info::refresh() {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    timestamp = ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

void su_info::check_db() {
    eval_uid = uid;
    get_db_settings(cfg);

    // Check multiuser settings
    switch (cfg[SU_MULTIUSER_MODE]) {
    case MULTIUSER_MODE_OWNER_ONLY:
        if (to_user_id(uid) != 0) {
            eval_uid = -1;
            access = NO_SU_ACCESS;
        }
        break;
    case MULTIUSER_MODE_OWNER_MANAGED:
        eval_uid = to_app_id(uid);
        break;
    case MULTIUSER_MODE_USER:
    default:
        break;
    }

    if (eval_uid > 0) {
        char query[256], *err;
        ssprintf(query, sizeof(query),
            "SELECT policy, logging, notification FROM policies "
            "WHERE uid=%d AND (until=0 OR until>%li)", eval_uid, time(nullptr));
        err = db_exec(query, [&](db_row &row) -> bool {
            access.policy = (policy_t) parse_int(row["policy"]);
            access.log = parse_int(row["logging"]);
            access.notify = parse_int(row["notification"]);
            LOGD("magiskdb: query policy=[%d] log=[%d] notify=[%d]\n",
                 access.policy, access.log, access.notify);
            return true;
        });
        db_err_cmd(err, return);
    }

    // We need to check our manager
    if (access.log || access.notify) {
        check_pkg_refresh();
        mgr_uid = get_manager(to_user_id(eval_uid), &mgr_pkg, true);
    }
}

bool uid_granted_root(int uid) {
    if (uid == AID_ROOT)
        return true;

    db_settings cfg;
    get_db_settings(cfg);

    // Check user root access settings
    switch (cfg[ROOT_ACCESS]) {
    case ROOT_ACCESS_DISABLED:
        return false;
    case ROOT_ACCESS_APPS_ONLY:
        if (uid == AID_SHELL)
            return false;
        break;
    case ROOT_ACCESS_ADB_ONLY:
        if (uid != AID_SHELL)
            return false;
        break;
    case ROOT_ACCESS_APPS_AND_ADB:
        break;
    }

    // Check multiuser settings
    switch (cfg[SU_MULTIUSER_MODE]) {
    case MULTIUSER_MODE_OWNER_ONLY:
        if (to_user_id(uid) != 0)
            return false;
        break;
    case MULTIUSER_MODE_OWNER_MANAGED:
        uid = to_app_id(uid);
        break;
    case MULTIUSER_MODE_USER:
    default:
        break;
    }

    bool granted = false;

    char query[256], *err;
    ssprintf(query, sizeof(query),
        "SELECT policy FROM policies WHERE uid=%d AND (until=0 OR until>%li)",
        uid, time(nullptr));
    err = db_exec(query, [&](db_row &row) -> bool {
        granted = parse_int(row["policy"]) == ALLOW;
        return true;
    });
    db_err_cmd(err, return false);

    return granted;
}

void prune_su_access() {
    cached.reset();
    vector<bool> app_no_list = get_app_no_list();
    vector<int> rm_uids;
    char query[256], *err;
    strscpy(query, "SELECT uid FROM policies", sizeof(query));
    err = db_exec(query, [&](db_row &row) -> bool {
        int uid = parse_int(row["uid"]);
        int app_id = to_app_id(uid);
        if (app_id >= AID_APP_START && app_id <= AID_APP_END) {
            int app_no = app_id - AID_APP_START;
            if (app_no >= app_no_list.size() || !app_no_list[app_no]) {
                // The app_id is no longer installed
                rm_uids.push_back(uid);
            }
        }
        return true;
    });
    db_err_cmd(err, return);

    for (int uid : rm_uids) {
        ssprintf(query, sizeof(query), "DELETE FROM policies WHERE uid == %d", uid);
        // Don't care about errors
        db_exec(query);
    }
}

static shared_ptr<su_info> get_su_info(unsigned uid) {
    if (uid == AID_ROOT) {
        auto info = make_shared<su_info>(uid);
        info->access = SILENT_SU_ACCESS;
        return info;
    }

    shared_ptr<su_info> info;
    {
        mutex_guard lock(cache_lock);
        if (!cached || cached->uid != uid || !cached->is_fresh())
            cached = make_shared<su_info>(uid);
        cached->refresh();
        info = cached;
    }

    mutex_guard lock = info->lock();

    if (info->access.policy == QUERY) {
        // Not cached, get data from database
        info->check_db();

        // If it's the manager, allow it silently
        if (to_app_id(info->uid) == to_app_id(info->mgr_uid)) {
            info->access = SILENT_SU_ACCESS;
            return info;
        }

        // Check su access settings
        switch (info->cfg[ROOT_ACCESS]) {
            case ROOT_ACCESS_DISABLED:
                LOGW("Root access is disabled!\n");
                info->access = NO_SU_ACCESS;
                break;
            case ROOT_ACCESS_ADB_ONLY:
                if (info->uid != AID_SHELL) {
                    LOGW("Root access limited to ADB only!\n");
                    info->access = NO_SU_ACCESS;
                }
                break;
            case ROOT_ACCESS_APPS_ONLY:
                if (info->uid == AID_SHELL) {
                    LOGW("Root access is disabled for ADB!\n");
                    info->access = NO_SU_ACCESS;
                }
                break;
            case ROOT_ACCESS_APPS_AND_ADB:
            default:
                break;
        }

        if (info->access.policy != QUERY)
            return info;

        // If still not determined, check if manager exists
        if (info->mgr_uid < 0) {
            info->access = NO_SU_ACCESS;
            return info;
        }
    }
    return info;
}

// Set effective uid back to root, otherwise setres[ug]id will fail if uid isn't root
static void set_identity(uid_t uid, const std::vector<uid_t> &groups) {
    if (seteuid(0)) {
        PLOGE("seteuid (root)");
    }
    gid_t gid;
    if (groups.size() > 0) {
        if (setgroups(groups.size(), groups.data())) {
            PLOGE("setgroups");
        }
        gid = groups[0];
    } else {
        gid = uid;
    }
    if (setresgid(gid, gid, gid)) {
        PLOGE("setresgid (%u)", uid);
    }
    if (setresuid(uid, uid, uid)) {
        PLOGE("setresuid (%u)", uid);
    }
}

// WK: added on 21/01/2024:
static void multiplexing(int infd, int outfd, int errfd, int log_fd)
{
    struct timeval tv;
    fd_set fds;
    int rin;
    int rout;
    int rerr;
    
    /* Wait 1 second for data arrival, then give up. */
    tv.tv_sec = 1;
    tv.tv_usec = 0;
	
	ssize_t inlen;
        ssize_t outlen;
	ssize_t errlen;
	int written;
	
	char input[ARG_MAX];
	char output[ARG_MAX];
	char err[ARG_MAX];
	

       FD_ZERO(&fds);
       FD_SET(STDIN_FILENO, &fds);
       FD_SET(outfd, &fds);
       FD_SET(errfd, &fds);
   
	while (1) {
            
      FD_ZERO(&fds);
      FD_SET(STDIN_FILENO, &fds);

      rin = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
			
     if (rin >= 1) {
	 memset(input, 0, sizeof(input));
		       
        if ((inlen = read(STDIN_FILENO, input, 4096)) > 0) {
               LOGD("input:%s", input);
              written =  write(infd, input, inlen);
             LOGD("written to infd %d", written);
             write(log_fd, input, inlen);
	  }
    }
		
   FD_ZERO(&fds);
   FD_SET(outfd, &fds);

    rout = select(outfd + 1, &fds, NULL, NULL, &tv);
		   
   if (rout >= 1) {
        LOGD("rout: %d", rout);
       memset(output, 0, sizeof(output));
			    
        if ((outlen = read(outfd, output, 4096)) > 0) {
	       written = write(STDOUT_FILENO, output, outlen);
		LOGD(" written to STDOUT_FILENO: %d", written);
               // tag indicating the init of output  (to be used by Magisk app to show the command output in green color)
                write(log_fd, "{", strlen("{"));
                write(log_fd, output, outlen); 
                write(log_fd, "\n", strlen("\n"));
                // tag indicating the the end of command's output  (to be used by Magisk app to show the command output in green color)
                write(log_fd, "}", strlen("}"));
                write(log_fd, "\n", strlen("\n"));

               // WK: added on 24/01/2024: this fixes the "exit" command issue:
		continue;				
        }
    }
	 
   FD_ZERO(&fds);
   FD_SET(errfd, &fds);
		
    rerr = select(errfd + 1, &fds, NULL, NULL, &tv); 

    if (rerr >= 1) {
         LOGD("reread: %d", rerr);

          memset(err, 0, sizeof(err));
		        
	   if ((errlen = read(errfd, err, 4096)) > 0) {
		  LOGD("error:%s", err);
	          written = write(STDERR_FILENO, err, errlen);
                  LOGD("written to STDERR_FILENO: %d", written);
      
                 // tag indicating the init of command's error  (to be used by Magisk app to show the command output in red color)
                 write(log_fd, "!", strlen("!")); 
                 write(log_fd, err, errlen);
                 write(log_fd, "\n", strlen("\n"));
                 // tag indicating the end of command's error  (to be used by Magisk app to show the command output in green color)
                 write(log_fd, "!", strlen("!")); 
                 write(log_fd, "\n", strlen("\n"));
               
                // WK: added on 24/01/2024: this fixes the "exit" command issue:
		continue;
                }
              
              // WK: added on 24/01/2024: this fixes the "exit" command issue:
	     FD_ZERO(&fds);
             FD_SET(STDIN_FILENO, &fds);
		
	    rin = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
			    
	    LOGD ("rin2: %d", rin);
	     
          if (rin == 0) {
	  // WK: added on 24/01/2024: this fixes the "exit" command issue: if there is no data on STDIN_FILENO, break out of the loop so the process continue its normal flow and call waitpid() and exit().
	   break;
          }	        
     }
  }
	
}

void su_daemon_handler(int client, const sock_cred *cred) {
    LOGD("su: request from uid=[%d], pid=[%d], client=[%d]\n", cred->uid, cred->pid, client);

    su_context ctx = {
        .info = get_su_info(cred->uid),
        .req = su_request(),
        .pid = cred->pid
    };

    // Read su_request
    if (xxread(client, &ctx.req, sizeof(su_req_base)) < 0
        || !read_string(client, ctx.req.shell)
        || !read_string(client, ctx.req.command)
        || !read_string(client, ctx.req.context)
        || !read_vector(client, ctx.req.gids)) {
        LOGW("su: remote process probably died, abort\n");
        ctx.info.reset();
        write_int(client, DENY);
        close(client);
        return;
    }

    // If still not determined, ask manager
    if (ctx.info->access.policy == QUERY) {
        int fd = app_request(ctx);
        if (fd < 0) {
            ctx.info->access.policy = DENY;
        } else {
            int ret = read_int_be(fd);
            ctx.info->access.policy = ret < 0 ? DENY : static_cast<policy_t>(ret);
            close(fd);
        }
    }

    if (ctx.info->access.log)
        app_log(ctx);
    else if (ctx.info->access.notify)
        app_notify(ctx);

    // Fail fast
    if (ctx.info->access.policy == DENY) {
        LOGW("su: request rejected (%u)\n", ctx.info->uid);
        ctx.info.reset();
        write_int(client, DENY);
        close(client);
        return;
    }

    // Fork a child root process
    //
    // The child process will need to setsid, open a pseudo-terminal
    // if needed, and eventually exec shell.
    // The parent process will wait for the result and
    // send the return code back to our client.

    if (int child = xfork(); child) {
        ctx.info.reset();

        // Wait result
        LOGD("su: waiting child pid=[%d]\n", child);
        int status, code;

        if (waitpid(child, &status, 0) > 0)
            code = WEXITSTATUS(status);
        else
            code = -1;

        LOGD("su: return code=[%d]\n", code);
        write(client, &code, sizeof(code));
        close(client);
        return;
    }

    LOGD("su: fork handler\n");

    // Abort upon any error occurred
    exit_on_error(true);

    // ack
    write_int(client, 0);

    // Become session leader
    xsetsid();

    // The FDs for each of the streams
    int infd = recv_fd(client);
    int outfd = recv_fd(client);
    int errfd = recv_fd(client);

    int is_tty = read_int(client);
    
    // App need a PTY
    if (is_tty) {
        string pts;
        string ptmx;
        auto magiskpts = get_magisk_tmp() + "/"s SHELLPTS;
        if (access(magiskpts.data(), F_OK)) {
            pts = "/dev/pts";
            ptmx = "/dev/ptmx";
        } else {
            pts = magiskpts;
            ptmx = magiskpts + "/ptmx";
        }
        int ptmx_fd = xopen(ptmx.data(), O_RDWR);
        grantpt(ptmx_fd);
        unlockpt(ptmx_fd);
        int pty_num = get_pty_num(ptmx_fd);
        if (pty_num < 0) {
            // Kernel issue? Fallback to /dev/pts
            close(ptmx_fd);
            pts = "/dev/pts";
            ptmx_fd = xopen("/dev/ptmx", O_RDWR);
            grantpt(ptmx_fd);
            unlockpt(ptmx_fd);
            pty_num = get_pty_num(ptmx_fd);
        }
        send_fd(client, ptmx_fd);
        close(ptmx_fd);

        string pts_slave = pts + "/" + to_string(pty_num);
        LOGD("su: pts_slave=[%s]\n", pts_slave.data());

        // Opening the TTY has to occur after the
        // fork() and setsid() so that it becomes
        // our controlling TTY and not the daemon's
        int ptsfd = xopen(pts_slave.data(), O_RDWR);

        if (infd < 0)
            infd = ptsfd;
        if (outfd < 0)
            outfd = ptsfd;
        if (errfd < 0)
            errfd = ptsfd;
    }

    // Swap out stdin, stdout, stderr
    xdup2(infd, STDIN_FILENO);
    xdup2(outfd, STDOUT_FILENO);
    xdup2(errfd, STDERR_FILENO);

    close(infd);
    close(outfd);
    close(errfd);
    close(client);

    int inputfd[2];
    int outputfd[2];
    int errorfd[2];

    int ptmx = -1;
    int ptsfd;
    char pts_slave[PATH_MAX];

    if (is_tty) {
        // We need a PTY. Get one.
        ptmx = pts_open(pts_slave, sizeof(pts_slave));
        
        if (ptmx < 0) {
            PLOGE("pts_open");
            //exit(-1);
         }
    } else {
        //WK: use pipes for normal apps
        pipe(inputfd);
        pipe(outputfd);
        pipe(errorfd);
    }
	
    char log_name[PATH_MAX];
    int log_fd = -1;
    char *logs_folder = "/debug_ramdisk/logs";
    struct timeval tv;
	
    gettimeofday(&tv, NULL);
	
    unsigned int current_time = (unsigned int)(tv.tv_sec);
	
    if (access("/sbin/magisk", F_OK) == 0) {
	logs_folder = "/sbin/logs";
    }
    
    mkdir(logs_folder, 0770);
    if (chown(logs_folder, ctx.info->mgr_uid, ctx.info->mgr_uid)) {
        PLOGE("chown (%s, %ld, %ld)", logs_folder, ctx.info->mgr_uid, ctx.info->mgr_uid);
        // WK: do not deny: the "REQUESTOR_DATA_PATH/files" folder may not exists.
		//deny(&ctx);
    }
	
    ssprintf(log_name, sizeof(log_name), "%s/%u-%u", logs_folder, cred->uid, current_time);
   
    log_fd = open(log_name, O_CREAT | O_APPEND | O_RDWR, 0666);
    if (log_fd < 0) {
        PLOGE("Open(log_fd)");
       // return -1;
    }
    chmod(log_fd, 0666);
	
    // WK: fork another process for I/O multiplexing
    pid_t pid = fork();

if (pid == 0) {
    if(is_tty) {
	setsid();
        // Opening the TTY has to occur after the
        // fork() and setsid() so that it becomes
        // our controlling TTY and not the daemon's
        ptsfd = open(pts_slave, O_RDWR);
        if (ptsfd == -1) {
            PLOGE("open(pts_slave) daemon");
            exit(-1);
        }
        
        infd  = ptsfd;
        outfd = ptsfd;
        errfd = ptsfd;
        	
        if (-1 == dup2(infd, STDIN_FILENO)) {
            PLOGE("dup2 child infd");
            exit(-1);
        }
	if (-1 == dup2(outfd, STDOUT_FILENO)) {
            PLOGE("dup2 child outfd");
            exit(-1);
        }
	if (-1 == dup2(errfd, STDERR_FILENO)) {
            PLOGE("dup2 child errfd");
            exit(-1);
        }
        
	close(infd);
	close(outfd);
	close(errfd);
    } else {
        if (-1 == dup2(inputfd[0], STDIN_FILENO)) {
            PLOGE("dup2 child infd");
            exit(-1);
	}
	if (-1 == dup2(outputfd[1], STDOUT_FILENO)) {
            PLOGE("dup2 child outfd");
	    exit(-1);
	}
	if (-1 == dup2(errorfd[1], STDERR_FILENO)) {
            PLOGE("dup2 child errfd");
            exit(-1);
        }
		
        close(inputfd[0]);
	close(inputfd[1]);
	close(outputfd[0]);
	close(outputfd[1]);
	close(errorfd[0]);
	close(errorfd[1]);
    }
    
    // Handle namespaces
     if (ctx.req.target == -1)
        ctx.req.target = ctx.pid;
    else if (ctx.req.target == 0)
        ctx.info->cfg[SU_MNT_NS] = NAMESPACE_MODE_GLOBAL;
    else if (ctx.info->cfg[SU_MNT_NS] == NAMESPACE_MODE_GLOBAL)
        ctx.info->cfg[SU_MNT_NS] = NAMESPACE_MODE_REQUESTER;
    switch (ctx.info->cfg[SU_MNT_NS]) {
        case NAMESPACE_MODE_GLOBAL:
            LOGD("su: use global namespace\n");
            break;
        case NAMESPACE_MODE_REQUESTER:
            LOGD("su: use namespace of pid=[%d]\n", ctx.req.target);
            if (switch_mnt_ns(ctx.req.target))
                LOGD("su: setns failed, fallback to global\n");
            break;
        case NAMESPACE_MODE_ISOLATE:
            LOGD("su: use new isolated namespace\n");
            switch_mnt_ns(ctx.req.target);
            xunshare(CLONE_NEWNS);
            xmount(nullptr, "/", nullptr, MS_PRIVATE | MS_REC, nullptr);
            break;
    }

    const char *argv[4] = { nullptr };

    argv[0] = ctx.req.login ? "-" : ctx.req.shell.data();

    size_t cmd_size;
    char *cmd;
    
    if (!ctx.req.command.empty()) {
        argv[1] = "-c";
        argv[2] = ctx.req.command.data();

        cmd = ctx.req.command.data();
        cmd_size = strlen(cmd) + 1;
	cmd[cmd_size] = '\n';
	write(log_fd, cmd, cmd_size);
	write(log_fd, "\n", 2);  
    }

    // Setup environment
    umask(022);
    char path[32];
    ssprintf(path, sizeof(path), "/proc/%d/cwd", ctx.pid);
    char cwd[4096];
    if (realpath(path, cwd, sizeof(cwd)) > 0)
        chdir(cwd);
    ssprintf(path, sizeof(path), "/proc/%d/environ", ctx.pid);
    auto env = full_read(path);
    clearenv();
    for (size_t pos = 0; pos < env.size(); ++pos) {
        putenv(env.data() + pos);
        pos = env.find_first_of('\0', pos);
        if (pos == std::string::npos)
            break;
    }
    if (!ctx.req.keepenv) {
        struct passwd *pw;
        pw = getpwuid(ctx.req.uid);
        if (pw) {
            setenv("HOME", pw->pw_dir, 1);
            setenv("USER", pw->pw_name, 1);
            setenv("LOGNAME", pw->pw_name, 1);
            setenv("SHELL", ctx.req.shell.data(), 1);
        }
    }

    // Unblock all signals
    sigset_t block_set;
    sigemptyset(&block_set);
    sigprocmask(SIG_SETMASK, &block_set, nullptr);
    if (!ctx.req.context.empty()) {
        auto f = xopen_file("/proc/self/attr/exec", "we");
        if (f) fprintf(f.get(), "%s", ctx.req.context.data());
    }
    set_identity(ctx.req.uid, ctx.req.gids);
    execvp(ctx.req.shell.data(), (char **) argv);
    fprintf(stderr, "Cannot execute %s: %s\n", ctx.req.shell.data(), strerror(errno));
    PLOGE("exec");
  } else { 
     int status, code;

     if (is_tty) {
        watch_sigwinch_async(STDOUT_FILENO, ptmx);
        pump_stdin_async(ptmx);
        pump_stdout_blocking(ptmx, log_fd);

        LOGD("Waiting for pid %d.", pid);
        waitpid(pid, &status, 0);
        
	code = WEXITSTATUS(status);
        exit(code);
     } else {
        close(inputfd[0]);
	close(outputfd[1]);
	close(errorfd[1]);
	multiplexing(inputfd[1], outputfd[0], errorfd[0], log_fd);
        
	LOGD("Waiting for pid %d.", pid);
        waitpid(pid, &status, 0);
       
        close(inputfd[1]);
	close(outputfd[0]);
	close(errorfd[0]);
        
	code = WEXITSTATUS(status);
        exit(code);
     }
  }
}
