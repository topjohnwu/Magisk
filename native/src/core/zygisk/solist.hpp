// Idea from https://github.com/LSPosed/NativeDetector/blob/master/app/src/main/jni/solist.cpp
// Modified by reveny

#pragma once

#include <xdl.h>

namespace SoList 
{
    class SoInfo {
    public:
#ifdef __LP64__
        inline static size_t solist_next_offset = 0x30;
        constexpr static size_t solist_realpath_offset = 0x1a8;
#else
        inline static size_t solist_next_offset = 0xa4;
        constexpr static size_t solist_realpath_offset = 0x174;
#endif

        inline static const char *(*get_realpath_sym)(SoInfo *) = nullptr;
        inline static const char *(*get_soname_sym)(SoInfo *) = nullptr;

        inline SoInfo *get_next() {
            return *(SoInfo **) ((uintptr_t) this + solist_next_offset);
        }

        inline const char *get_path() {
            return get_realpath_sym ? get_realpath_sym(this) : ((std::string *) ((uintptr_t) this + solist_realpath_offset))->c_str();
        }

        inline const char *get_name() {
            return get_soname_sym ? get_soname_sym(this) : *((const char **) ((uintptr_t) this + solist_realpath_offset - sizeof(void *)));
        }

        void nullify_name() {
            const char** name = (const char**)get_soname_sym(this);

            static const char* empty_string = "";
            *name = reinterpret_cast<const char *>(&empty_string);
        }

        void nullify_path() {
            const char** name = (const char**)get_realpath_sym(this);

            static const char* empty_string = "";
            *name = reinterpret_cast<const char *>(&empty_string);
        }
    };

    static SoInfo *solist = nullptr;
    static SoInfo *somain = nullptr;

    template<typename T>
    inline T *getStaticPointer(void* handle, const char* name) {
        auto *addr = reinterpret_cast<T **>(xdl_dsym(handle, name, nullptr));
        return addr == nullptr ? nullptr : *addr;
    }

    static void NullifySoName(const char* target_name) {
        for (auto *iter = solist; iter; iter = iter->get_next()) {
            if (iter->get_name() && iter->get_path() && strstr(iter->get_path(), target_name)) {
                ZLOGD("solist_nullify: [%s] [%s]\n", iter->get_name(), iter->get_path());
                //iter->nullify_name(); // crash, not needed
                iter->nullify_path();
            }
        }
    }

    static bool Initialize() {
        #if defined(__LP64__)
            void* dl_linker = xdl_open("/system/bin/linker64", XDL_TRY_FORCE_LOAD);
        #else
            void* dl_linker = xdl_open("/system/bin/linker", XDL_TRY_FORCE_LOAD);
        #endif

        if (dl_linker == nullptr) {
            ZLOGE("SoList::Initialize(): Error dlopen linker for initializing SoList %s\n", dlerror());
            return false;
        }

        solist = getStaticPointer<SoInfo>(dl_linker, "__dl__ZL6solist");
        somain = getStaticPointer<SoInfo>(dl_linker, "__dl__ZL6somain");

        if (solist != nullptr && somain != nullptr) {
            SoInfo::get_realpath_sym = reinterpret_cast<decltype(SoInfo::get_realpath_sym)>(xdl_dsym(dl_linker, "__dl__ZNK6soinfo12get_realpathEv", nullptr));
            SoInfo::get_soname_sym = reinterpret_cast<decltype(SoInfo::get_soname_sym)>(xdl_dsym(dl_linker, "__dl__ZNK6soinfo10get_sonameEv", nullptr));
            auto vsdo = getStaticPointer<SoInfo>(dl_linker, "__dl__ZL4vdso");

            for (size_t i = 0; i < 1024 / sizeof(void *); i++) {
                auto *possible_next = *(void **) ((uintptr_t) solist + i * sizeof(void *));
                if (possible_next == somain || (vsdo != nullptr && possible_next == vsdo)) {
                    SoInfo::solist_next_offset = i * sizeof(void *);

                    break;
                }
            }

            return (SoInfo::get_realpath_sym != nullptr && SoInfo::get_soname_sym != nullptr);
        } else {
            ZLOGE("Unable to find linker symbols solist:%p somain:%p\n", solist, somain);
        }

        return false;
    }
}
