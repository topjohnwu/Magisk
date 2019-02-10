#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <vector>

#include <logcat.h>
#include <utils.h>
#include <logging.h>
#include <magisk.h>

static std::vector<const char *> log_cmd;
static pthread_mutex_t event_lock = PTHREAD_MUTEX_INITIALIZER;
static time_t LAST_TIMESTAMP = 0;

bool logcat_started = false;

struct log_listener {
	bool enable = false;
	bool (*filter)(const char *);
	BlockingQueue<std::string> queue;
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
	const char *buffers[] = { "main", "events", "crash" };
	for (auto b : buffers) {
		if (exec_command_sync(MIRRDIR "/system/bin/logcat", "-b", b, "-d", "-f", "/dev/null") == 0) {
			log_cmd.push_back("-b");
			log_cmd.push_back(b);
		}
	}
	chmod("/dev/null", 0666);
	log_cmd.insert(log_cmd.end(), { "-v", "threadtime", "-s", "am_proc_start", "Magisk" });
#ifdef MAGISK_DEBUG
	log_cmd.push_back("*:F");
#endif
	log_cmd.push_back(nullptr);
}

static bool test_logcat() {
	int test = exec_command_sync(MIRRDIR "/system/bin/logcat", "-d", "-f", "/dev/null");
	chmod("/dev/null", 0666);
	return test == 0;
}

static void *logcat_gobbler(void *) {
	int log_pid;
	char line[4096];
	struct tm tm{};
	time_t prev;

	// Set tm year info
	time_t now = time(nullptr);
	localtime_r(&now, &tm);

	while (true) {
		prev = 0;
		exec_t exec {
			.fd = -1,
			.argv = log_cmd.data()
		};
		log_pid = exec_command(exec);
		FILE *logs = fdopen(exec.fd, "r");
		while (fgets(line, sizeof(line), logs)) {
			if (line[0] == '-')
				continue;
			// Parse timestamp
			strptime(line, "%m-%d %H:%M:%S", &tm);
			now = mktime(&tm);
			if (now < prev) {
				/* Log timestamps should be monotonic increasing, if this happens,
				 * it means that we occur the super rare case: crossing year boundary
				 * (e.g 2019 -> 2020). Reset and reparse timestamp */
				now = time(nullptr);
				localtime_r(&now, &tm);
				strptime(line, "%m-%d %H:%M:%S", &tm);
				now = mktime(&tm);
			}
			// Skip old logs
			if (now < LAST_TIMESTAMP)
				continue;
			LAST_TIMESTAMP = prev = now;
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

		LOGI("logcat: unexpected output EOF");

		// Wait a few seconds and retry
		sleep(2);
		if (!test_logcat()) {
			// Cancel all events and terminate
			logcat_started = false;
			for (auto &event : events)
				event.queue.cancel();
			return nullptr;
		}
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
	if (!test_logcat())
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
