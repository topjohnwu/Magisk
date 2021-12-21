/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "ProcessCallStack"
// #define LOG_NDEBUG 0

#include <utils/ProcessCallStack.h>

#include <dirent.h>
#include <unistd.h>

#include <memory>

#include <utils/Printer.h>

namespace android {

enum {
    // Max sizes for various dynamically generated strings
    MAX_TIME_STRING = 64,
    MAX_PROC_PATH = 1024,

    // Dump related prettiness constants
    IGNORE_DEPTH_CURRENT_THREAD = 2,
};

static const char* CALL_STACK_PREFIX = "  ";
static const char* PATH_THREAD_NAME = "/proc/self/task/%d/comm";
static const char* PATH_SELF_TASK = "/proc/self/task";

static void dumpProcessHeader(Printer& printer, pid_t pid, const char* timeStr) {
    if (timeStr == nullptr) {
        ALOGW("%s: timeStr was NULL", __FUNCTION__);
        return;
    }

    char path[PATH_MAX];
    char procNameBuf[MAX_PROC_PATH];
    char* procName = nullptr;
    FILE* fp;

    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    if ((fp = fopen(path, "r"))) {
        procName = fgets(procNameBuf, sizeof(procNameBuf), fp);
        fclose(fp);
    }

    if (!procName) {
        procName = const_cast<char*>("<unknown>");
    }

    printer.printLine();
    printer.printLine();
    printer.printFormatLine("----- pid %d at %s -----", pid, timeStr);
    printer.printFormatLine("Cmd line: %s", procName);
}

static void dumpProcessFooter(Printer& printer, pid_t pid) {
    printer.printLine();
    printer.printFormatLine("----- end %d -----", pid);
    printer.printLine();
}

static String8 getThreadName(pid_t tid) {
    char path[PATH_MAX];
    char* procName = nullptr;
    char procNameBuf[MAX_PROC_PATH];
    FILE* fp;

    snprintf(path, sizeof(path), PATH_THREAD_NAME, tid);
    if ((fp = fopen(path, "r"))) {
        procName = fgets(procNameBuf, sizeof(procNameBuf), fp);
        fclose(fp);
    } else {
        ALOGE("%s: Failed to open %s", __FUNCTION__, path);
    }

    if (procName == nullptr) {
        // Reading /proc/self/task/%d/comm failed due to a race
        return String8::format("[err-unknown-tid-%d]", tid);
    }

    // Strip ending newline
    strtok(procName, "\n");

    return String8(procName);
}

static String8 getTimeString(struct tm tm) {
    char timestr[MAX_TIME_STRING];
    // i.e. '2013-10-22 14:42:05'
    strftime(timestr, sizeof(timestr), "%F %T", &tm);

    return String8(timestr);
}

/*
 * Implementation of ProcessCallStack
 */
ProcessCallStack::ProcessCallStack() {
}

ProcessCallStack::ProcessCallStack(const ProcessCallStack& rhs) :
        mThreadMap(rhs.mThreadMap),
        mTimeUpdated(rhs.mTimeUpdated) {
}

ProcessCallStack::~ProcessCallStack() {
}

void ProcessCallStack::clear() {
    mThreadMap.clear();
    mTimeUpdated = tm();
}

void ProcessCallStack::update() {
    std::unique_ptr<DIR, decltype(&closedir)> dp(opendir(PATH_SELF_TASK), closedir);
    if (dp == nullptr) {
        ALOGE("%s: Failed to update the process's call stacks: %s",
              __FUNCTION__, strerror(errno));
        return;
    }

    pid_t selfPid = getpid();

    clear();

    // Get current time.
    {
        time_t t = time(nullptr);
        struct tm tm;
        localtime_r(&t, &tm);

        mTimeUpdated = tm;
    }

    /*
     * Each tid is a directory inside of /proc/self/task
     * - Read every file in directory => get every tid
     */
    dirent* ep;
    while ((ep = readdir(dp.get())) != nullptr) {
        pid_t tid = -1;
        sscanf(ep->d_name, "%d", &tid);

        if (tid < 0) {
            // Ignore '.' and '..'
            ALOGV("%s: Failed to read tid from %s/%s",
                  __FUNCTION__, PATH_SELF_TASK, ep->d_name);
            continue;
        }

        ssize_t idx = mThreadMap.add(tid, ThreadInfo());
        if (idx < 0) { // returns negative error value on error
            ALOGE("%s: Failed to add new ThreadInfo: %s",
                  __FUNCTION__, strerror(-idx));
            continue;
        }

        ThreadInfo& threadInfo = mThreadMap.editValueAt(static_cast<size_t>(idx));

        /*
         * Ignore CallStack::update and ProcessCallStack::update for current thread
         * - Every other thread doesn't need this since we call update off-thread
         */
        int ignoreDepth = (selfPid == tid) ? IGNORE_DEPTH_CURRENT_THREAD : 0;

        // Update thread's call stacks
        threadInfo.callStack.update(ignoreDepth, tid);

        // Read/save thread name
        threadInfo.threadName = getThreadName(tid);

        ALOGV("%s: Got call stack for tid %d (size %zu)",
              __FUNCTION__, tid, threadInfo.callStack.size());
    }
}

void ProcessCallStack::log(const char* logtag, android_LogPriority priority,
                           const char* prefix) const {
    LogPrinter printer(logtag, priority, prefix, /*ignoreBlankLines*/false);
    print(printer);
}

void ProcessCallStack::print(Printer& printer) const {
    /*
     * Print the header/footer with the regular printer.
     * Print the callstack with an additional two spaces as the prefix for legibility.
     */
    PrefixPrinter csPrinter(printer, CALL_STACK_PREFIX);
    printInternal(printer, csPrinter);
}

void ProcessCallStack::printInternal(Printer& printer, Printer& csPrinter) const {
    dumpProcessHeader(printer, getpid(),
                      getTimeString(mTimeUpdated).string());

    for (size_t i = 0; i < mThreadMap.size(); ++i) {
        pid_t tid = mThreadMap.keyAt(i);
        const ThreadInfo& threadInfo = mThreadMap.valueAt(i);
        const String8& threadName = threadInfo.threadName;

        printer.printLine("");
        printer.printFormatLine("\"%s\" sysTid=%d", threadName.string(), tid);

        threadInfo.callStack.print(csPrinter);
    }

    dumpProcessFooter(printer, getpid());
}

void ProcessCallStack::dump(int fd, int indent, const char* prefix) const {

    if (indent < 0) {
        ALOGW("%s: Bad indent (%d)", __FUNCTION__, indent);
        return;
    }

    FdPrinter printer(fd, static_cast<unsigned int>(indent), prefix);
    print(printer);
}

String8 ProcessCallStack::toString(const char* prefix) const {

    String8 dest;
    String8Printer printer(&dest, prefix);
    print(printer);

    return dest;
}

size_t ProcessCallStack::size() const {
    return mThreadMap.size();
}

}; //namespace android
