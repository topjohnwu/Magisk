#include <utils.hpp>

#include "magiskvbmeta.hpp"

using namespace std;

static void usage(char *arg0) {
    fprintf(stderr,
R"EOF(MagiskVbmeta - Vbmeta Image Modification Tool

Usage: %s <action> [args...]

Supported actions:
  test <vbmetaimg>
    Test <vbmetaimg>'s status
    Return values:
    0:stock    1:patched    2:other    4:invalid

  size <vbmetaimg>
    Print padded <vbmetaimg> size

  patch <origvbmetaimg> [outvbmetaimg]
    Patch <origvbmetaimg> to [outbootimg], or
    new-vbmeta.img if not specified.
    All disable flags will be set.)EOF", arg0);

    fprintf(stderr, "\n\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    cmdline_logging();
    umask(0);

    if (argc < 3)
        usage(argv[0]);

    string_view action(argv[1]);

    if (argc > 2 && action == "test") {
        return test(argv[2]);
    } else if (argc > 2 && action == "size") {
        return size(argv[2]);
    } else if (argc > 2 && action == "patch") {
        return patch(argv[2], argv[3] ? argv[3] : NEW_VBMETA);
    } else {
        usage(argv[0]);
    }
    return 0;
}
