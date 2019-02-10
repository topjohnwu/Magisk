#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <vector>

#include <logcat.h>
#include <utils.h>
#include <logging.h>
#include <magisk.h>

static std::vector<const char *> log_cmd, clear_cmd;
static pthread_mutex_t event_lock = PTHREAD_MUTEX_INITIALIZER;
bool logcat_started = false;

struct log_listener {
	bool enable = false;
	BlockingQueue<std::string> queue;
	bool (*filter)(const char *);
};

static struct log_listener events[] = {
	{	/* HIDE_EVENT */
		.filter = [](auto log) -> bool { return strstr(log, "am_proc_start") != nullptr; }
	},
	{	/* LOG_EVENT */
		.filter = [](auto log) -> bool { return !strstr(log, "am_proc_start"); }
	}
};

static void init_args() {
	// Construct cmdline
	log_cmd.push_back(MIRRDIR "/system/bin/logcat");
	// Test whether these buffers actually works
	const char *b[] = { "main", "events", "crash" };
	for (auto &buffer : b) {
		if (exec_command_sync(MIRRDIR "/system/bin/logcat", "-b", buffer, "-d", "-f", "/dev/null") == 0) {
			log_cmd.push_back("-b");
			log_cmd.push_back(buffer);
		}
	}
	chmod("/dev/null", 0666);
	clear_cmd = log_cmd;
	log_cmd.insert(log_cmd.end(), { "-v", "threadtime", "-s", "am_proc_start", "Magisk" });
#ifdef MAGISK_DEBUG
	log_cmd.push_back("*:F");
#endif
	log_cmd.push_back(nullptr);

	clear_cmd.push_back("-c");
	clear_cmd.push_back(nullptr);
}

static void *logcat_gobbler(void *) {
	int log_pid;
	char line[4096];
	while (true) {
		// Start logcat
		exec_t exec {
			.fd = -1,
			.argv = log_cmd.data()
		};
		log_pid = exec_command(exec);
		FILE *logs = fdopen(exec.fd, "r");
		while (fgets(line, sizeof(line), logs)) {
			if (line[0] == '-')
				continue;
			pthread_mutex_lock(&event_lock);
			for (auto &event : events) {
				if (event.enable && event.filter(line))
					event.queue.emplace_back(line);
			}
			pthread_mutex_unlock(&event_lock);
		}

		fclose(logs);
		kill(log_pid, SIGTERM);
		waitpid(log_pid, nullptr, 0);

		LOGI("magisklogd: logcat output EOF");
		// Clear buffer
		exec_command_sync(clear_cmd.data());
	}
}

static void *log_writer(void *) {
	rename(LOGFILE, LOGFILE ".bak");
	FILE *log = xfopen(LOGFILE, "ae");
	setbuf(log, nullptr);
	auto &queue = start_logging(LOG_EVENT);
	while (true) {
		fprintf(log, "%s", queue.take().c_str());
	}
}

BlockingQueue<std::string> &start_logging(logcat_event event) {
	pthread_mutex_lock(&event_lock);
	events[event].enable = true;
	pthread_mutex_unlock(&event_lock);
	return events[event].queue;
}

void stop_logging(logcat_event event) {
	pthread_mutex_lock(&event_lock);
	events[event].enable = false;
	events[event].queue.clear();
	pthread_mutex_unlock(&event_lock);
}

bool start_logcat() {
	if (logcat_started)
		return true;
	int test = exec_command_sync(MIRRDIR "/system/bin/logcat", "-d", "-f", "/dev/null");
	chmod("/dev/null", 0666);
	if (test != 0)
		return false;
	init_args();
	pthread_t t;
	pthread_create(&t, nullptr, log_writer, nullptr);
	pthread_detach(t);
	pthread_create(&t, nullptr, logcat_gobbler, nullptr);
	pthread_detach(t);
	logcat_started = true;
	return true;
}

