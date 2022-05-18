#include <base.hpp>
#include <magisk.hpp>
#include <daemon.hpp>
#include <db.hpp>

using namespace std;

// These functions will be called on every single zygote process specialization and su request,
// so performance is absolutely critical. Most operations should either have its result cached
// or simply skipped unless necessary.

static atomic<ino_t> pkg_xml_ino = 0;

// pkg_lock protects mgr_app_id and mgr_pkg
static pthread_mutex_t pkg_lock = PTHREAD_MUTEX_INITIALIZER;
static int mgr_app_id = -1;
static string *mgr_pkg;

bool need_pkg_refresh() {
    struct stat st{};
    stat("/data/system/packages.xml", &st);
    ino_t ino = st.st_ino;
    if (pkg_xml_ino.compare_exchange_strong(ino, st.st_ino)) {
        // Packages have not changed
        return false;
    } else {
        mutex_guard g(pkg_lock);
        mgr_app_id = -1;
        return true;
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

int get_manager(int user_id, std::string *pkg) {
    mutex_guard g(pkg_lock);

    char app_path[128];
    struct stat st{};
    if (mgr_pkg == nullptr)
        default_new(mgr_pkg);

    int app_id = mgr_app_id;
    if (app_id > 0) {
        // Just need to check whether the app is installed in the user
        snprintf(app_path, sizeof(app_path), "%s/%d/%s", APP_DATA_DIR, user_id, mgr_pkg->data());
        if (access(app_path, F_OK) == 0) {
            if (pkg) *pkg = *mgr_pkg;
            return user_id * AID_USER_OFFSET + app_id;
        } else {
            goto not_found;
        }
    } else {
        db_strings str;
        get_db_strings(str, SU_MANAGER);

        if (!str[SU_MANAGER].empty()) {
            // App is repackaged
            snprintf(app_path, sizeof(app_path),
                     "%s/%d/%s", APP_DATA_DIR, user_id, str[SU_MANAGER].data());
            if (stat(app_path, &st) == 0) {
                mgr_pkg->swap(str[SU_MANAGER]);
            }
        } else {
            // Check the original package name
            snprintf(app_path, sizeof(app_path), "%s/%d/" JAVA_PACKAGE_NAME, APP_DATA_DIR, user_id);
            if (stat(app_path, &st) == 0) {
                *mgr_pkg = JAVA_PACKAGE_NAME;
            } else {
                goto not_found;
            }
        }
    }

    mgr_app_id = to_app_id(st.st_uid);
    if (pkg) *pkg = *mgr_pkg;
    return st.st_uid;

not_found:
    LOGE("su: cannot find manager\n");
    if (pkg)
        pkg->clear();
    return -1;
}
