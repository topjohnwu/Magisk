#include <libgen.h>
#include <sys/stat.h>
#include <string.h>  // Added for null check utilities

#include "include/core.hpp"

using namespace std;

struct Applet {
    string_view name;
    int (*fn)(int, char *[]);
};

constexpr Applet applets[] = {
    { "su", su_client_main },
    { "resetprop", resetprop_main },
};

constexpr Applet private_applets[] = {
    { "zygisk", zygisk_main },
};

// Helper function: Safe null/empty check for C strings (prevents SIGTRAP)
static inline bool is_empty_or_null(const char* str) {
    return str == nullptr || *str == '\0';
}

int main(int argc, char *argv[]) {
    // Fix 1: Hardened argc/argv validation (prevents access to null/empty argv)
    if (argc < 1 || argv == nullptr || is_empty_or_null(argv[0])) {
        fprintf(stderr, "magisk: invalid invocation (argc=%d, argv[0]=%p)\n", argc, argv ? argv[0] : nullptr);
        return 1;
    }

    cmdline_logging();
    init_argv0(argc, argv);

    Utf8CStr argv0 = basename(argv[0]);

    umask(0);

    if (argv[0][0] == '\0') {
        // When argv[0] is an empty string, we're calling private applets
        // Fix 2: Validate argc >=2 AND argv[1] is not null/empty
        if (argc < 2 || is_empty_or_null(argv[1])) {
            fprintf(stderr, "magisk: empty private applet name (argc=%d)\n", argc);
            return 1;
        }
        --argc;
        ++argv;
        // Fix 3: Re-check argv[0] after shifting (prevents null access)
        if (is_empty_or_null(argv[0])) {
            fprintf(stderr, "magisk: private applet name is empty\n");
            return 1;
        }
        bool applet_found = false;
        for (const auto &app : private_applets) {
            if (argv[0] == app.name) {
                applet_found = true;
                return app.fn(argc, argv);
            }
        }
        if (!applet_found) {
            fprintf(stderr, "%s: applet not found\n", argv[0]);
        }
        return 1;
    }

    if (argv0 == "magisk" || argv0 == "magisk32" || argv0 == "magisk64") {
        // Fix 4: Validate argv[1] before accessing (prevents null/empty applet name)
        if (argc > 1 && !is_empty_or_null(argv[1]) && argv[1][0] != '-') {
            // Calling applet with "magisk [applet] args..."
            --argc;
            ++argv;
            // Fix 5: Re-validate argv[0] after shifting
            if (is_empty_or_null(argv[0])) {
                fprintf(stderr, "magisk: applet name is empty\n");
                return 1;
            }
            argv0 = argv[0];
        } else {
            return magisk_main(argc, argv);
        }
    }

    // Fix 6: Ensure argv0 is valid before final applet lookup
    if (is_empty_or_null(argv0.c_str())) {
        fprintf(stderr, "magisk: applet name is empty\n");
        return 1;
    }

    for (const auto &app : applets) {
        if (argv0 == app.name) {
            return app.fn(argc, argv);
        }
    }
    fprintf(stderr, "%s: applet not found\n", argv0.c_str());
    return 1;
}