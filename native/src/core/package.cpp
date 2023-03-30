#include <base.hpp>
#include <magisk.hpp>
#include <daemon.hpp>
#include <db.hpp>
#include <flags.h>

#include "core.hpp"

using namespace std;

#define ENFORCE_SIGNATURE (!MAGISK_DEBUG)

// These functions will be called on every single zygote process specialization and su request,
// so performance is absolutely critical. Most operations should either have its result cached
// or simply skipped unless necessary.

atomic<ino_t> pkg_xml_ino = 0;
static atomic_flag skip_mgr_check;

static pthread_mutex_t pkg_lock = PTHREAD_MUTEX_INITIALIZER;
// pkg_lock protects all following variables
static int mgr_app_id = -1;
static string *mgr_pkg;
static string *mgr_cert;
static int stub_apk_fd = -1;
static const string *default_cert;

void check_pkg_refresh() {
    struct stat st{};
    if (stat("/data/system/packages.xml", &st) == 0 &&
        pkg_xml_ino.exchange(st.st_ino) != st.st_ino) {
        skip_mgr_check.clear();
        skip_pkg_rescan.clear();
    }
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
    string stub_path = MAGISKTMP + "/stub.apk";
    stub_apk_fd = xopen(stub_path.data(), O_RDONLY | O_CLOEXEC);
    unlink(stub_path.data());
    default_cert = new string(read_certificate(stub_apk_fd));
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

int get_manager(int user_id, string *pkg, bool install) {
    mutex_guard g(pkg_lock);

    char app_path[128];
    struct stat st{};
    if (mgr_pkg == nullptr)
        default_new(mgr_pkg);
    if (mgr_cert == nullptr)
        default_new(mgr_cert);

    auto check_dyn = [&](int u) -> bool {
#if ENFORCE_SIGNATURE
        ssprintf(app_path, sizeof(app_path),
            "%s/%d/%s/dyn/current.apk", APP_DATA_DIR, u, mgr_pkg->data());
        int dyn = open(app_path, O_RDONLY | O_CLOEXEC);
        if (dyn < 0) {
            LOGW("pkg: no dyn APK, ignore\n");
            return false;
        }
        bool mismatch = default_cert && read_certificate(dyn, MAGISK_VER_CODE) != *default_cert;
        close(dyn);
        if (mismatch) {
            LOGE("pkg: dyn APK signature mismatch: %s\n", app_path);
            clear_pkg(mgr_pkg->data(), u);
            return false;
        }
#endif
        return true;
    };

    if (skip_mgr_check.test_and_set()) {
        if (mgr_app_id >= 0) {
            // Just need to check whether the app is installed in the user
            const char *name = mgr_pkg->empty() ? JAVA_PACKAGE_NAME : mgr_pkg->data();
            ssprintf(app_path, sizeof(app_path), "%s/%d/%s", APP_DATA_DIR, user_id, name);
            if (access(app_path, F_OK) == 0) {
                // Always check dyn signature for repackaged app
                if (!mgr_pkg->empty() && !check_dyn(user_id))
                    goto ignore;
                if (pkg) *pkg = name;
                return user_id * AID_USER_OFFSET + mgr_app_id;
            } else {
                goto not_found;
            }
        }
    } else {
        // Here, we want to actually find the manager app and cache the results.
        // This means that we check all users, not just the requested user.
        // Certificates are also verified to prevent manipulation.

        db_strings str;
        get_db_strings(str, SU_MANAGER);

        vector<int> users;
        bool collected = false;

        auto collect_users = [&] {
            if (collected)
                return;
            collected = true;
            auto data_dir = xopen_dir(APP_DATA_DIR);
            if (!data_dir)
                return;
            dirent *entry;
            while ((entry = xreaddir(data_dir.get()))) {
                // Only collect users not requested as we've already checked it
                if (int u = parse_int(entry->d_name); u >= 0 && u != user_id)
                    users.push_back(parse_int(entry->d_name));
            }
        };

        if (!str[SU_MANAGER].empty()) {
            // Check the repackaged package name

            bool invalid = false;
            auto check_stub_apk = [&](int u) -> bool {
                ssprintf(app_path, sizeof(app_path),
                         "%s/%d/%s", APP_DATA_DIR, u, str[SU_MANAGER].data());
                if (stat(app_path, &st) == 0) {
                    int app_id = to_app_id(st.st_uid);

                    string apk = find_apk_path(str[SU_MANAGER].data());
                    int fd = xopen(apk.data(), O_RDONLY | O_CLOEXEC);
                    string cert = read_certificate(fd);
                    close(fd);

                    // Verify validity
                    if (str[SU_MANAGER] == *mgr_pkg) {
                        if (app_id != mgr_app_id || cert != *mgr_cert) {
                            // app ID or cert should never change
                            LOGE("pkg: repackaged APK signature invalid: %s\n", apk.data());
                            uninstall_pkg(mgr_pkg->data());
                            invalid = true;
                            install = true;
                            return false;
                        }
                    }

                    mgr_pkg->swap(str[SU_MANAGER]);
                    mgr_app_id = app_id;
                    mgr_cert->swap(cert);
                    return true;
                }
                return false;
            };

            if (check_stub_apk(user_id)) {
                if (!check_dyn(user_id))
                    goto ignore;
                if (pkg) *pkg = *mgr_pkg;
                return st.st_uid;
            }
            if (!invalid) {
                collect_users();
                for (int u : users) {
                    if (check_stub_apk(u)) {
                        // Found repackaged app, but not installed in the requested user
                        goto not_found;
                    }
                    if (invalid)
                        break;
                }
            }

            // Repackaged app not found, fall through
        }

        // Check the original package name

        bool invalid = false;
        auto check_apk = [&](int u) -> bool {
            ssprintf(app_path, sizeof(app_path), "%s/%d/" JAVA_PACKAGE_NAME, APP_DATA_DIR, u);
            if (stat(app_path, &st) == 0) {
#if ENFORCE_SIGNATURE
                string apk = find_apk_path(JAVA_PACKAGE_NAME);
                int fd = xopen(apk.data(), O_RDONLY | O_CLOEXEC);
                string cert = read_certificate(fd, MAGISK_VER_CODE);
                close(fd);
                if (default_cert && cert != *default_cert) {
                    // Found APK with invalid signature, force replace with stub
                    LOGE("pkg: APK signature mismatch: %s\n", apk.data());
                    uninstall_pkg(JAVA_PACKAGE_NAME);
                    invalid = true;
                    install = true;
                    return false;
                }
#endif
                mgr_pkg->clear();
                mgr_cert->clear();
                mgr_app_id = to_app_id(st.st_uid);
                return true;
            }
            return false;
        };

        if (check_apk(user_id)) {
            if (pkg) *pkg = JAVA_PACKAGE_NAME;
            return st.st_uid;
        }
        if (!invalid) {
            collect_users();
            for (int u : users) {
                if (check_apk(u)) {
                    // Found app, but not installed in the requested user
                    goto not_found;
                }
                if (invalid)
                    break;
            }
        }
    }

    // No manager app is found, clear all cached value
    mgr_app_id = -1;
    mgr_pkg->clear();
    mgr_cert->clear();
    if (install)
        install_stub();

not_found:
    LOGW("pkg: cannot find %s for user=[%d]\n",
         mgr_pkg->empty() ? JAVA_PACKAGE_NAME : mgr_pkg->data(), user_id);
ignore:
    if (pkg) pkg->clear();
    return -1;
}
