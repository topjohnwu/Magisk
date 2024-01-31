#include <base.hpp>
#include <consts.hpp>
#include <core.hpp>
#include <db.hpp>
#include <flags.h>

using namespace std;
using rust::Vec;

#define ENFORCE_SIGNATURE (!MAGISK_DEBUG)

// These functions will be called on every single zygote process specialization and su request,
// so performance is absolutely critical. Most operations should either have its result cached
// or simply skipped unless necessary.

static pthread_mutex_t pkg_lock = PTHREAD_MUTEX_INITIALIZER;
// pkg_lock protects all following variables
static int stub_apk_fd = -1;
static int repackaged_app_id = -1; // Only used by dyn
static string repackaged_pkg;
static Vec<uint8_t> repackaged_cert;
static Vec<uint8_t> trusted_cert;
static map<int, pair<string, time_t>> user_to_check;
enum status {
    INSTALLED,
    NO_INSTALLED,
    CERT_MISMATCH,
};

static bool operator==(const Vec<uint8_t> &a, const Vec<uint8_t> &b) {
    return a.size() == b.size() && memcmp(a.data(), b.data(), a.size()) == 0;
}

// app_id = app_no + AID_APP_START
// app_no range: [0, 9999]
vector<bool> get_app_no_list() {
    vector<bool> list;
    auto data_dir = xopen_dir(APP_DATA_DIR);
    if (!data_dir)
        return list;
    dirent *entry;
    while ((entry = xreaddir(data_dir.get()))) {
        // For each user
        int dfd = xopenat(dirfd(data_dir.get()), entry->d_name, O_RDONLY);
        if (auto dir = xopen_dir(dfd)) {
            while ((entry = xreaddir(dir.get()))) {
                // For each package
                struct stat st{};
                xfstatat(dfd, entry->d_name, &st, 0);
                int app_id = to_app_id(st.st_uid);
                if (app_id >= AID_APP_START && app_id <= AID_APP_END) {
                    int app_no = app_id - AID_APP_START;
                    if (list.size() <= app_no) {
                        list.resize(app_no + 1);
                    }
                    list[app_no] = true;
                }
            }
        } else {
            close(dfd);
        }
    }
    return list;
}

void preserve_stub_apk() {
    mutex_guard g(pkg_lock);
    string stub_path = get_magisk_tmp() + "/stub.apk"s;
    stub_apk_fd = xopen(stub_path.data(), O_RDONLY | O_CLOEXEC);
    unlink(stub_path.data());
    trusted_cert = read_certificate(stub_apk_fd, MAGISK_VER_CODE);
    lseek(stub_apk_fd, 0, SEEK_SET);
}

static void install_stub() {
    if (stub_apk_fd < 0)
        return;
    struct stat st{};
    fstat(stub_apk_fd, &st);
    char apk[] = "/data/stub.apk";
    int dfd = xopen(apk, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    xsendfile(dfd, stub_apk_fd, nullptr, st.st_size);
    lseek(stub_apk_fd, 0, SEEK_SET);
    close(dfd);
    install_apk(apk);
}

static status check_dyn(int user, string &pkg) {
    struct stat st{};
    char apk[PATH_MAX];
    ssprintf(apk, sizeof(apk),
             "%s/%d/%s/dyn/current.apk", APP_DATA_DIR, user, pkg.data());
    int dyn = open(apk, O_RDONLY | O_CLOEXEC);
    if (dyn < 0) {
        LOGW("pkg: no dyn APK, ignore\n");
        return NO_INSTALLED;
    }
    auto cert = read_certificate(dyn, MAGISK_VER_CODE);
    fstat(dyn, &st);
    close(dyn);

    if (cert.empty() || cert != trusted_cert) {
        LOGE("pkg: dyn APK signature mismatch: %s\n", apk);
#if ENFORCE_SIGNATURE
        clear_pkg(pkg.data(), user);
        return CERT_MISMATCH;
#endif
    }

    repackaged_app_id = to_app_id(st.st_uid);
    user_to_check[user] = make_pair(apk, st.st_ctim.tv_sec);
    return INSTALLED;
}

static status check_stub(int user, string &pkg) {
    struct stat st{};
    byte_array<PATH_MAX> buf;
    find_apk_path(pkg, buf);
    string apk((const char *) buf.buf(), buf.sz());
    int fd = xopen(apk.data(), O_RDONLY | O_CLOEXEC);
    if (fd < 0)
        return NO_INSTALLED;
    auto cert = read_certificate(fd, -1);
    fstat(fd, &st);
    close(fd);

    if (cert.empty() || (pkg == repackaged_pkg && cert != repackaged_cert)) {
        LOGE("pkg: repackaged APK signature invalid: %s\n", apk.data());
        uninstall_pkg(pkg.data());
        return CERT_MISMATCH;
    }

    repackaged_pkg.swap(pkg);
    repackaged_cert.swap(cert);
    user_to_check[user] = make_pair(apk, st.st_ctim.tv_sec);

    return INSTALLED;
}

static status check_orig(int user) {
    struct stat st{};
    byte_array<PATH_MAX> buf;
    find_apk_path(JAVA_PACKAGE_NAME, buf);
    string apk((const char *) buf.buf(), buf.sz());
    int fd = xopen(apk.data(), O_RDONLY | O_CLOEXEC);
    if (fd < 0)
        return NO_INSTALLED;
    auto cert = read_certificate(fd, MAGISK_VER_CODE);
    fstat(fd, &st);
    close(fd);

    if (cert.empty() || cert != trusted_cert) {
        LOGE("pkg: APK signature mismatch: %s\n", apk.data());
#if ENFORCE_SIGNATURE
        uninstall_pkg(JAVA_PACKAGE_NAME);
        return CERT_MISMATCH;
#endif
    }

    user_to_check[user] = make_pair(apk, st.st_ctim.tv_sec);
    return INSTALLED;
}

static int get_pkg_uid(int user, const char *pkg) {
    char path[PATH_MAX];
    struct stat st{};
    ssprintf(path, sizeof(path), "%s/%d/%s", APP_DATA_DIR, user, pkg);
    if (stat(path, &st) == 0) {
        return st.st_uid;
    }
    return -1;
}

int get_manager(int user, string *pkg, bool install) {
    mutex_guard g(pkg_lock);

    struct stat st{};
    const auto &[path, time] = user_to_check[user];
    if (stat(path.data(), &st) == 0 && st.st_ctim.tv_sec == time) {
        // no APK
        if (path == "/data/system/packages.xml") {
            if (install) install_stub();
            if (pkg) pkg->clear();
            return -1;
        }
        // dyn APK is still the same
        if (path.starts_with(APP_DATA_DIR)) {
            if (pkg) *pkg = repackaged_pkg;
            return user * AID_USER_OFFSET + repackaged_app_id;
        }
        // stub APK is still the same
        if (!repackaged_pkg.empty()) {
            if (check_dyn(user, repackaged_pkg) == INSTALLED) {
                if (pkg) *pkg = repackaged_pkg;
                return user * AID_USER_OFFSET + repackaged_app_id;
            } else {
                if (pkg) pkg->clear();
                return -1;
            }
        }
        // orig APK is still the same
        int uid = get_pkg_uid(user, JAVA_PACKAGE_NAME);
        if (uid < 0) {
            if (pkg) pkg->clear();
            return -1;
        } else {
            if (pkg) *pkg = JAVA_PACKAGE_NAME;
            return uid;
        }
    }

    db_strings str;
    get_db_strings(str, SU_MANAGER);

    if (!str[SU_MANAGER].empty()) {
        switch (check_stub(user, str[SU_MANAGER])) {
            case INSTALLED:
                if (check_dyn(user, repackaged_pkg) == INSTALLED) {
                    if (pkg) *pkg = repackaged_pkg;
                    return user * AID_USER_OFFSET + repackaged_app_id;
                } else {
                    if (pkg) pkg->clear();
                    return -1;
                }
            case CERT_MISMATCH:
                install = true;
            case NO_INSTALLED:
                break;
        }
    }

    repackaged_pkg.clear();
    repackaged_cert.clear();
    switch (check_orig(user)) {
        case INSTALLED: {
            int uid = get_pkg_uid(user, JAVA_PACKAGE_NAME);
            if (uid < 0) {
                if (pkg) pkg->clear();
                return -1;
            } else {
                if (pkg) *pkg = JAVA_PACKAGE_NAME;
                return uid;
            }
        }
        case CERT_MISMATCH:
            install = true;
        case NO_INSTALLED:
            break;
    }

    auto xml = "/data/system/packages.xml";
    stat(xml, &st);
    user_to_check[user] = make_pair(xml, st.st_ctim.tv_sec);

    if (install) install_stub();
    if (pkg) pkg->clear();
    return -1;
}
