# liblog -> logd

The data that liblog sends to logd is represented below.

    struct {
        android_log_header_t header;
        union {
           struct {
                char     prio;
                char     tag[...];
                char     message[...];
            } string;
            struct {
                android_event_header_t event_header;
                android_event_*_t      payload[...];
            } binary;
        };
    };

where the embedded structs are defined as:

    struct android_log_header_t {
        uint8_t id;
        uint16_t tid;
        log_time realtime;
    };

    struct log_time {
        uint32_t tv_sec = 0;
        uint32_t tv_nsec = 0;
    }

    struct android_event_header_t {
        int32_t tag;
    };

    struct android_event_list_t {
        int8_t type;  // EVENT_TYPE_LIST
        int8_t element_count;
    };

    struct android_event_float_t {
        int8_t type;  // EVENT_TYPE_FLOAT
        float data;
    };

    struct android_event_int_t {
        int8_t type;   // EVENT_TYPE_INT
        int32_t data;
    } android_event_int_t;

    struct android_event_long_t {
        int8_t type;   // EVENT_TYPE_LONG
        int64_t data;
    };

    struct android_event_string_t {
        int8_t type;     // EVENT_TYPE_STRING;
        int32_t length;
        char data[];
    };

The payload, excluding the header, has a max size of LOGGER_ENTRY_MAX_PAYLOAD.

## header

The header is added immediately before sending the log message to logd.

## `string` payload

The `string` part of the union is for normal buffers (main, system, radio, etc) and consists of a
single character priority, followed by a variable length null terminated string for the tag, and
finally a variable length null terminated string for the message.

This payload is used for the `__android_log_buf_write()` family of functions.

## `binary` payload

The `binary` part of the union is for binary buffers (events, security, etc) and consists of an
android_event_header_t struct followed by a variable number of android_event_*_t
(android_event_list_t, android_event_int_t, etc) structs.

If multiple android_event_*_t elements are present, then they must be in a list and the first
element in payload must be an android_event_list_t.

This payload is used for the `__android_log_bwrite()` family of functions. It is additionally used
for `android_log_write_list()` and the related functions that manipulate event lists.

# logd -> liblog

logd sends a `logger_entry` struct to liblog followed by the payload. The payload is identical to
the payloads defined above. The max size of the entire message from logd is LOGGER_ENTRY_MAX_LEN.
