#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <set>
#include <unistd.h>
#include <base.hpp>

struct MountInfo {
    unsigned int id;
    unsigned int parent;
    dev_t device;
    std::string root;
    std::string target;
    std::string vfs_option;
    struct {
        unsigned int shared;
        unsigned int master;
        unsigned int propagate_from;
    } optional;
    std::string type;
    std::string source;
    std::string fs_option;
};

static std::vector<MountInfo> ParseMountInfo(const char *pid) {
    char buf[PATH_MAX] = {};
    ssprintf(buf, sizeof(buf), "/proc/%s/mountinfo", pid);
    auto mount_info = xopen_file(buf, "r");
    char *line = nullptr;
    run_finally free_line([&line] { free(line); });
    size_t len = 0;
    ssize_t nread;

    std::vector<MountInfo> result;

    while ((nread = getline(&line, &len, mount_info.get())) != -1) {
        if (line[nread - 1] == '\n')
            line[nread - 1] = '\0';
        int root_start = 0, root_end = 0;
        int target_start = 0, target_end = 0;
        int vfs_option_start = 0, vfs_option_end = 0;
        int type_start = 0, type_end = 0;
        int source_start = 0, source_end = 0;
        int fs_option_start = 0, fs_option_end = 0;
        int optional_start = 0, optional_end = 0;
        unsigned int id, parent, maj, min;
        sscanf(line,
               "%u "           // (1) id
               "%u "           // (2) parent
               "%u:%u "        // (3) maj:min
               "%n%*s%n "      // (4) mountroot
               "%n%*s%n "      // (5) target
               "%n%*s%n"       // (6) vfs options (fs-independent)
               "%n%*[^-]%n - " // (7) optional fields
               "%n%*s%n "      // (8) FS type
               "%n%*s%n "      // (9) source
               "%n%*s%n",      // (10) fs options (fs specific)
               &id, &parent, &maj, &min, &root_start, &root_end, &target_start,
               &target_end, &vfs_option_start, &vfs_option_end,
               &optional_start, &optional_end, &type_start, &type_end,
               &source_start, &source_end, &fs_option_start, &fs_option_end);
        std::string_view line_view(line, nread - 1);

        auto root = line_view.substr(root_start, root_end - root_start);
        auto target = line_view.substr(target_start, target_end - target_start);
        auto vfs_option =
                line_view.substr(vfs_option_start, vfs_option_end - vfs_option_start);
        ++optional_start;
        --optional_end;
        auto optional = line_view.substr(
                optional_start,
                optional_end - optional_start > 0 ? optional_end - optional_start : 0);

        auto type = line_view.substr(type_start, type_end - type_start);
        auto source = line_view.substr(source_start, source_end - source_start);
        auto fs_option =
                line_view.substr(fs_option_start, fs_option_end - fs_option_start);

        unsigned int shared = 0;
        unsigned int master = 0;
        unsigned int propagate_from = 0;
        if (auto pos = optional.find("shared:"); pos != std::string_view::npos) {
            shared = strtoul(optional.data() + pos + 7, nullptr, 10);
        }
        if (auto pos = optional.find("master:"); pos != std::string_view::npos) {
            master = strtoul(optional.data() + pos + 7, nullptr, 10);
        }
        if (auto pos = optional.find("propagate_from:");
                pos != std::string_view::npos) {
            propagate_from = strtoul(optional.data() + pos + 15, nullptr, 10);
        }

        result.emplace_back(MountInfo{
                .id = id,
                .parent = parent,
                .device = static_cast<dev_t>(makedev(maj, min)),
                .root = std::string(root),
                .target = std::string(target),
                .vfs_option = std::string(vfs_option),
                .optional =
                        {
                                .shared = shared,
                                .master = master,
                                .propagate_from = propagate_from,
                        },
                .type = std::string(type),
                .source = std::string(source),
                .fs_option = std::string(fs_option),
        });
    }
    return result;
}
