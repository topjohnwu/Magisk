#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include <base.hpp>

#include "prop.hpp"

using namespace std;

/* ***********************************************************************
 * Auto generated header and field definitions compiled from
 * https://android.googlesource.com/platform/system/core/+/master/init/persistent_properties.proto
 * Generated with Nanopb: https://github.com/nanopb/nanopb
 * ***********************************************************************/

/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.3 */

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
struct PersistentProperties {
    pb_callback_t properties;
};

struct PersistentProperties_PersistentPropertyRecord {
    pb_callback_t name;
    bool has_value;
    char value[92];
};

/* Initializer values for message structs */
#define PersistentProperties_init_default        {{{NULL}, NULL}}
#define PersistentProperties_PersistentPropertyRecord_init_default {{{NULL}, NULL}, false, ""}
#define PersistentProperties_init_zero           {{{NULL}, NULL}}
#define PersistentProperties_PersistentPropertyRecord_init_zero {{{NULL}, NULL}, false, ""}

/* Field tags (for use in manual encoding/decoding) */
#define PersistentProperties_properties_tag      1
#define PersistentProperties_PersistentPropertyRecord_name_tag 1
#define PersistentProperties_PersistentPropertyRecord_value_tag 2

/* Struct field encoding specification for nanopb */
#define PersistentProperties_FIELDLIST(X, a) \
X(a, CALLBACK, REPEATED, MESSAGE,  properties,        1)
#define PersistentProperties_CALLBACK pb_default_field_callback
#define PersistentProperties_DEFAULT NULL
#define PersistentProperties_properties_MSGTYPE PersistentProperties_PersistentPropertyRecord

#define PersistentProperties_PersistentPropertyRecord_FIELDLIST(X, a) \
X(a, CALLBACK, OPTIONAL, STRING,   name,              1) \
X(a, STATIC,   OPTIONAL, STRING,   value,             2)
#define PersistentProperties_PersistentPropertyRecord_CALLBACK pb_default_field_callback
#define PersistentProperties_PersistentPropertyRecord_DEFAULT NULL

extern const pb_msgdesc_t PersistentProperties_msg;
extern const pb_msgdesc_t PersistentProperties_PersistentPropertyRecord_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define PersistentProperties_fields &PersistentProperties_msg
#define PersistentProperties_PersistentPropertyRecord_fields &PersistentProperties_PersistentPropertyRecord_msg

/* Maximum encoded size of messages (where known) */
/* PersistentProperties_size depends on runtime parameters */
/* PersistentProperties_PersistentPropertyRecord_size depends on runtime parameters */

PB_BIND(PersistentProperties, PersistentProperties, AUTO)

PB_BIND(PersistentProperties_PersistentPropertyRecord, PersistentProperties_PersistentPropertyRecord, AUTO)

/* ***************************
 * End of auto generated code
 * ***************************/

bool use_pb = false;

static bool name_decode(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    string &name = *static_cast<string *>(*arg);
    name.resize(stream->bytes_left);
    return pb_read(stream, (pb_byte_t *)(name.data()), stream->bytes_left);
}

static bool name_encode(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
    return pb_encode_tag_for_field(stream, field) &&
           pb_encode_string(stream, (const pb_byte_t *) *arg, strlen((const char *) *arg));
}

static bool prop_decode(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    PersistentProperties_PersistentPropertyRecord prop{};
    string name;
    prop.name.funcs.decode = name_decode;
    prop.name.arg = &name;
    if (!pb_decode(stream, &PersistentProperties_PersistentPropertyRecord_msg, &prop))
        return false;
    auto cb = static_cast<prop_cb*>(*arg);
    cb->exec(std::move(name), prop.value);
    return true;
}

static bool prop_encode(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
    PersistentProperties_PersistentPropertyRecord prop{};
    prop.name.funcs.encode = name_encode;
    prop.has_value = true;
    auto &list = *static_cast<prop_list *>(*arg);
    for (auto &p : list) {
        if (!pb_encode_tag_for_field(stream, field))
            return false;
        prop.name.arg = (void *) p.first.data();
        strcpy(prop.value, p.second.data());
        if (!pb_encode_submessage(stream, &PersistentProperties_PersistentPropertyRecord_msg, &prop))
            return false;
    }
    return true;
}

static bool write_callback(pb_ostream_t *stream, const uint8_t *buf, size_t count) {
    int fd = (intptr_t)stream->state;
    return xwrite(fd, buf, count) == count;
}

static pb_ostream_t create_ostream(const char *filename) {
    int fd = creat(filename, 0644);
    pb_ostream_t o = {
        .callback = write_callback,
        .state = (void*)(intptr_t)fd,
        .max_size = SIZE_MAX,
        .bytes_written = 0,
    };
    return o;
}

static void pb_getprop(prop_cb *prop_cb) {
    LOGD("resetprop: decode with protobuf [" PERSISTENT_PROPERTY_DIR "/persistent_properties]\n");
    PersistentProperties props = {};
    props.properties.funcs.decode = prop_decode;
    props.properties.arg = prop_cb;
    auto m = mmap_data(PERSISTENT_PROPERTY_DIR "/persistent_properties");
    pb_istream_t stream = pb_istream_from_buffer(m.buf, m.sz);
    pb_decode(&stream, &PersistentProperties_msg, &props);
}

static bool file_getprop(const char *name, char *value) {
    char path[4096];
    snprintf(path, sizeof(path), PERSISTENT_PROPERTY_DIR "/%s", name);
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0)
        return false;
    LOGD("resetprop: read prop from [%s]\n", path);
    value[read(fd, value, PROP_VALUE_MAX - 1)] = '\0';  // Null terminate the read value
    close(fd);
    return value[0] != '\0';
}

void persist_getprops(prop_cb *prop_cb) {
    if (use_pb) {
        pb_getprop(prop_cb);
    } else {
        auto dir = open_dir(PERSISTENT_PROPERTY_DIR);
        if (!dir) return;
        for (dirent *entry; (entry = xreaddir(dir.get()));) {
            char value[PROP_VALUE_MAX];
            if (file_getprop(entry->d_name, value))
                prop_cb->exec(entry->d_name, value);
        }
    }
}

struct match_prop_name : prop_cb {
    explicit match_prop_name(const char *name) : _name(name) { value[0] = '\0'; }
    void exec(string &&name, const char *val) override {
        if (name == _name)
            strcpy(value, val);
    }
    char value[PROP_VALUE_MAX];
private:
    const char *_name;
};

string persist_getprop(const char *name) {
    if (use_pb) {
        auto prop = match_prop_name(name);
        pb_getprop(&prop);
        if (prop.value[0]) {
            LOGD("resetprop: getprop (persist) [%s]: [%s]\n", name, prop.value);
            return prop.value;
        }
    } else {
        // Try to read from file
        char value[PROP_VALUE_MAX];
        if (file_getprop(name, value)) {
            LOGD("resetprop: getprop (persist) [%s]: [%s]\n", name, value);
            return value;
        }
    }
    return string();
}

bool persist_deleteprop(const char *name) {
    if (use_pb) {
        prop_list list;
        prop_collector collector(list);
        persist_getprops(&collector);

        for (auto it = list.begin(); it != list.end(); ++it) {
            if (it->first == name) {
                list.erase(it);
                // Dump the props back
                PersistentProperties props{};
                pb_ostream_t ostream = create_ostream(PERSISTENT_PROPERTY_DIR
                        "/persistent_properties.tmp");
                props.properties.funcs.encode = prop_encode;
                props.properties.arg = &list;
                LOGD("resetprop: encode with protobuf [" PERSISTENT_PROPERTY_DIR
                             "/persistent_properties.tmp]\n");
                if (!pb_encode(&ostream, &PersistentProperties_msg, &props))
                    return false;
                clone_attr(PERSISTENT_PROPERTY_DIR "/persistent_properties",
                           PERSISTENT_PROPERTY_DIR "/persistent_properties.tmp");
                rename(PERSISTENT_PROPERTY_DIR "/persistent_properties.tmp",
                       PERSISTENT_PROPERTY_DIR "/persistent_properties");
                return true;
            }
        }
        return false;
    } else {
        char path[4096];
        snprintf(path, sizeof(path), PERSISTENT_PROPERTY_DIR "/%s", name);
        if (unlink(path) == 0) {
            LOGD("resetprop: unlink [%s]\n", path);
            return true;
        }
    }
    return false;
}
