#include <sys/mount.h>

#include <consts.hpp>
#include <base.hpp>
#include <sys/vfs.h>

#include "init.hpp"

using namespace std;

void MagiskInit::redirect_second_stage() const noexcept {
    // Patch init binary
    int src = xopen("/init", O_RDONLY);
    int dest = xopen("/data/init", O_CREAT | O_WRONLY, 0);
    {
        mmap_data init("/init");
        for (size_t off : init.patch(INIT_PATH, REDIR_PATH)) {
            LOGD("Patch @ %08zX [" INIT_PATH "] -> [" REDIR_PATH "]\n", off);
        }
        write(dest, init.buf(), init.sz());
        fclone_attr(src, dest);
        close(dest);
        close(src);
    }
    xmount("/data/init", "/init", nullptr, MS_BIND, nullptr);
}
